/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <purple.h>

#include <QCoreApplication>

#include <kwallet.h>

#include "purplekwallet.h"

/******************************************************************************
 * Globals
 *****************************************************************************/
static QCoreApplication *qCoreApp = NULL;
static PurpleCredentialProvider *instance = NULL;

#define PURPLE_KWALLET_DOMAIN (g_quark_from_static_string("purple-kwallet"))
#define PURPLE_KWALLET_WALLET_NAME (KWallet::Wallet::NetworkWallet())

struct _PurpleKWalletProvider {
	PurpleCredentialProvider parent;

	PurpleKWalletPlugin::Engine *engine;
};

G_DEFINE_DYNAMIC_TYPE(PurpleKWalletProvider, purple_kwallet_provider,
                      PURPLE_TYPE_CREDENTIAL_PROVIDER)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static QString
purple_kwallet_get_ui_name(void) {
	PurpleUi *ui = NULL;
	QString ui_name = NULL;

	ui = purple_core_get_ui();
	if(PURPLE_IS_UI(ui)) {
		ui_name = purple_ui_get_name(ui);
	}

	if(ui_name.isEmpty()) {
		ui_name = "libpurple";
	}

	return ui_name;
}

static QString
purple_kwallet_provider_account_key(PurpleAccount *account) {
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);

	return QString(purple_account_get_protocol_id(account)) + ":" +
	               purple_contact_info_get_username(info);
}

/******************************************************************************
 * Request Implementation
 *****************************************************************************/
PurpleKWalletPlugin::Request::Request(QString key, GTask *task) {
	this->key = key;
	this->task = G_TASK(g_object_ref(G_OBJECT(task)));
}

PurpleKWalletPlugin::Request::~Request(void) {
	g_clear_object(&this->task);
}

/******************************************************************************
 * ReadRequest Implementation
 *****************************************************************************/
PurpleKWalletPlugin::ReadRequest::ReadRequest(QString key, GTask *task) : PurpleKWalletPlugin::Request(key, task) {
}

void
PurpleKWalletPlugin::ReadRequest::execute(KWallet::Wallet *wallet) {
	QString password;
	int result = 0;
	bool missing;

	missing = KWallet::Wallet::keyDoesNotExist(PURPLE_KWALLET_WALLET_NAME,
	                                           purple_kwallet_get_ui_name(),
	                                           key);

	if(missing) {
		g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, 0,
		                        "no password stored");

		g_clear_object(&this->task);

		return;
	}

	result = wallet->readPassword(this->key, password);

	if(result != 0) {
		g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, result,
		                        _("failed to read password, kwallet responded "
		                          "with error code %d"), result);
	} else {
		gchar *c_password = g_strdup(password.toUtf8().constData());
		g_task_return_pointer(this->task, c_password, g_free);
	}

	g_clear_object(&this->task);
}

void
PurpleKWalletPlugin::ReadRequest::cancel(QString reason) {
	g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, 0,
	                        _("failed to read password: %s"),
	                        reason.toUtf8().constData());

	g_clear_object(&this->task);
}

/******************************************************************************
 * WriteRequest Implementation
 *****************************************************************************/
PurpleKWalletPlugin::WriteRequest::WriteRequest(QString key, GTask *task, QString password) : PurpleKWalletPlugin::Request(key, task) {
	this->password = password;
}

void
PurpleKWalletPlugin::WriteRequest::execute(KWallet::Wallet *wallet) {
	int result;

	result = wallet->writePassword(this->key, this->password);

	if(result != 0) {
		g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, result,
		                        _("failed to write password, kwallet "
		                          "responded with error code %d"), result);
	} else {
		g_task_return_boolean(this->task, TRUE);
	}

	g_clear_object(&this->task);
}

void
PurpleKWalletPlugin::WriteRequest::cancel(QString reason) {
	g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, 0,
	                        _("failed to write password: %s"),
	                        reason.toUtf8().constData());

	g_clear_object(&this->task);
}

/******************************************************************************
 * ClearRequest Implementation
 *****************************************************************************/
PurpleKWalletPlugin::ClearRequest::ClearRequest(QString key, GTask *task) : PurpleKWalletPlugin::Request(key, task) {
}

void
PurpleKWalletPlugin::ClearRequest::execute(KWallet::Wallet *wallet) {
	int result;

	result = wallet->removeEntry(this->key);

	if(result != 0) {
		g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, result,
		                        _("failed to clear password, kwallet "
		                          "responded with error code %d"), result);
	} else {
		g_task_return_boolean(this->task, TRUE);
	}

	g_clear_object(&this->task);
}

void
PurpleKWalletPlugin::ClearRequest::cancel(QString reason) {
	g_task_return_new_error(this->task, PURPLE_KWALLET_DOMAIN, 0,
	                        _("failed to clear password: %s"),
	                        reason.toUtf8().constData());

	g_clear_object(&this->task);
}

/******************************************************************************
 * Engine Implementation
 *****************************************************************************/
PurpleKWalletPlugin::Engine::Engine(void) {
	this->queue = QQueue<PurpleKWalletPlugin::Request *>();

	this->wallet = NULL;

	this->connected = false;
	this->failed = false;
	this->externallyClosed = false;
}

PurpleKWalletPlugin::Engine::~Engine(void) {
	this->close();
}

void
PurpleKWalletPlugin::Engine::open(void) {
	purple_debug_misc("kwallet-provider", "attempting to open wallet");

	if(this->connected) {
		purple_debug_misc("kwallet-provider", "wallet already opened");

		return;
	}

	// Reset our externallyClosed and failed states.
	this->externallyClosed = false;
	this->failed = false;

	// No need  to check this pointer as an async open always returns non-null.
	this->wallet = KWallet::Wallet::openWallet(PURPLE_KWALLET_WALLET_NAME,
	                                           0,
	                                           KWallet::Wallet::Asynchronous);

	this->failed |= !QObject::connect(this->wallet, SIGNAL(walletOpened(bool)),
	                                  SLOT(opened(bool)));
	this->failed |= !QObject::connect(this->wallet, SIGNAL(walletClosed(void)),
	                                  SLOT(closed()));

	if(this->failed) {
		purple_debug_error("kwallet-provider",
		                   "Failed to connect KWallet signals");
	}
}

void
PurpleKWalletPlugin::Engine::close(void) {
	while(!this->queue.isEmpty()) {
		PurpleKWalletPlugin::Request *request = this->queue.dequeue();

		request->cancel("wallet is closing");

		delete request;
	}

	if(this->wallet != NULL) {
		delete this->wallet;
		this->wallet = NULL;
	}

	this->connected = false;
	this->failed = false;
}

void
PurpleKWalletPlugin::Engine::enqueue(PurpleKWalletPlugin::Request *request) {
	this->queue.enqueue(request);

	processQueue();
}

void
PurpleKWalletPlugin::Engine::opened(bool opened) {
	QString folder_name;

	if(!opened) {
		purple_debug_error("kwallet-provider", "failed to open wallet");

		delete this->wallet;
		this->wallet = NULL;

		this->connected = false;
		this->failed = true;

		return;
	}

	// Handle the case where the wallet opened signal connected, but the wallet
	// closed signal failed to connect.
	if(this->failed) {
		purple_debug_error("kwallet-provider",
		                   "wallet opened, but failed to connect the wallet "
		                   "closed signal");
		return;
	}

	this->connected = true;

	// setup our folder
	folder_name = purple_kwallet_get_ui_name();
	if(!this->wallet->hasFolder(folder_name)) {
		if(!this->wallet->createFolder(folder_name)) {
			purple_debug_error("kwallet-provider",
			                   "failed to create folder %s in wallet.",
			                   folder_name.toUtf8().constData());
			this->failed = true;
		}
	}

	if(!this->failed && !this->wallet->setFolder(folder_name)) {
		purple_debug_error("kwallet-provider", "failed to set folder to %s",
		                   folder_name.toUtf8().constData());
		this->failed = true;
	}

	purple_debug_misc("kwallet-provider", "successfully opened the wallet");

	processQueue();
}

void
PurpleKWalletPlugin::Engine::closed(void) {
	purple_debug_misc("kwallet-provider", "the wallet was closed externally");

	this->externallyClosed = true;
	this->close();
}

void
PurpleKWalletPlugin::Engine::processQueue() {
	if(this->externallyClosed && this->queue.isEmpty() == false) {
		this->open();
	} else if(this->connected || this->failed) {
		while(!this->queue.isEmpty()) {
			PurpleKWalletPlugin::Request *request = this->queue.dequeue();

			if(this->failed) {
				request->cancel(_("failed to open kwallet"));
			} else {
				request->execute(this->wallet);
			}

			delete request;
		}
	}
}

/******************************************************************************
 * PurpleCredentialProvider Implementation
 *****************************************************************************/
static void
purple_kwallet_provider_activate(PurpleCredentialProvider *provider) {
	PurpleKWalletProvider *kwallet_provider = NULL;

	kwallet_provider = PURPLE_KWALLET_PROVIDER(provider);

	kwallet_provider->engine->open();
}

static void
purple_kwallet_provider_deactivate(PurpleCredentialProvider *provider) {
	PurpleKWalletProvider *kwallet_provider = NULL;

	kwallet_provider = PURPLE_KWALLET_PROVIDER(provider);

	kwallet_provider->engine->close();
}

static void
purple_kwallet_read_password_async(PurpleCredentialProvider *provider,
                                   PurpleAccount *account,
                                   GCancellable *cancellable,
                                   GAsyncReadyCallback callback,
                                   gpointer data)
{
	PurpleKWalletProvider *kwallet_provider = NULL;
	PurpleKWalletPlugin::ReadRequest *request = NULL;
	GTask *task = NULL;
	QString key;

	key = purple_kwallet_provider_account_key(account);

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	request = new PurpleKWalletPlugin::ReadRequest(key, task);

	kwallet_provider = PURPLE_KWALLET_PROVIDER(provider);
	kwallet_provider->engine->enqueue(request);

	g_clear_object(&task);
}

static gchar *
purple_kwallet_read_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                    GAsyncResult *result, GError **error)
{
	return (gchar *)g_task_propagate_pointer(G_TASK(result), error);
}

static void
purple_kwallet_write_password_async(PurpleCredentialProvider *provider,
                                    PurpleAccount *account,
                                    const gchar *password,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback,
                                    gpointer data)
{
	PurpleKWalletProvider *kwallet_provider = NULL;
	PurpleKWalletPlugin::WriteRequest *request = NULL;
	GTask *task = NULL;
	QString key;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	key = purple_kwallet_provider_account_key(account);

	request = new PurpleKWalletPlugin::WriteRequest(key, task, password);

	kwallet_provider = PURPLE_KWALLET_PROVIDER(provider);
	kwallet_provider->engine->enqueue(request);

	g_clear_object(&task);
}

static gboolean
purple_kwallet_write_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                     GAsyncResult *result, GError **error)
{
	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
purple_kwallet_clear_password_async(PurpleCredentialProvider *provider,
                                    PurpleAccount *account,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback,
                                    gpointer data)
{
	PurpleKWalletProvider *kwallet_provider = NULL;
	PurpleKWalletPlugin::ClearRequest *request = NULL;
	GTask *task = NULL;
	QString key;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	key = purple_kwallet_provider_account_key(account);

	request = new PurpleKWalletPlugin::ClearRequest(key, task);

	kwallet_provider = PURPLE_KWALLET_PROVIDER(provider);
	kwallet_provider->engine->enqueue(request);

	g_clear_object(&task);
}

static gboolean
purple_kwallet_clear_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                     GAsyncResult *result, GError **error)
{
	return g_task_propagate_boolean(G_TASK(result), error);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_kwallet_provider_dispose(GObject *obj) {
	PurpleKWalletProvider *provider = PURPLE_KWALLET_PROVIDER(obj);

	if(provider->engine != NULL) {
		provider->engine->close();
	}

	G_OBJECT_CLASS(purple_kwallet_provider_parent_class)->dispose(obj);
}

static void
purple_kwallet_provider_finalize(GObject *obj) {
	PurpleKWalletProvider *provider = PURPLE_KWALLET_PROVIDER(obj);

	if(provider->engine != NULL) {
		delete provider->engine;
		provider->engine = NULL;
	}

	G_OBJECT_CLASS(purple_kwallet_provider_parent_class)->finalize(obj);
}

static void
purple_kwallet_provider_init(PurpleKWalletProvider *provider) {
	provider->engine = new PurpleKWalletPlugin::Engine();
}

static void
purple_kwallet_provider_class_init(PurpleKWalletProviderClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleCredentialProviderClass *provider_class = NULL;

	provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);

	obj_class->dispose = purple_kwallet_provider_dispose;
	obj_class->finalize = purple_kwallet_provider_finalize;

	provider_class->activate = purple_kwallet_provider_activate;
	provider_class->deactivate = purple_kwallet_provider_deactivate;
	provider_class->read_password_async = purple_kwallet_read_password_async;
	provider_class->read_password_finish = purple_kwallet_read_password_finish;
	provider_class->write_password_async = purple_kwallet_write_password_async;
	provider_class->write_password_finish =
		purple_kwallet_write_password_finish;
	provider_class->clear_password_async = purple_kwallet_clear_password_async;
	provider_class->clear_password_finish =
		purple_kwallet_clear_password_finish;
}

static void
purple_kwallet_provider_class_finalize(G_GNUC_UNUSED PurpleKWalletProviderClass *klass) {
}

/******************************************************************************
 * API
 *****************************************************************************/
static PurpleCredentialProvider *
purple_kwallet_provider_new(void) {
	return PURPLE_CREDENTIAL_PROVIDER(g_object_new(
		PURPLE_KWALLET_TYPE_PROVIDER,
		"id", "kwallet",
		"name", _("KWallet"),
		"description", _("A credentials management application for the KDE "
		                 "Software Compilation desktop environment"),
		NULL
	));
}

/******************************************************************************
 * Plugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
kwallet_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return GPLUGIN_PLUGIN_INFO(purple_plugin_info_new(
		"id",           "keyring-kwallet",
		"name",         N_("KWallet"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Keyring"),
		"summary",      "KWallet Keyring Plugin",
		"description",  N_("This plugin will store passwords in KWallet."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		                PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	));
}

static gboolean
kwallet_load(GPluginPlugin *plugin, GError **error) {
	PurpleCredentialManager *manager = NULL;

	purple_kwallet_provider_register_type(G_TYPE_MODULE(plugin));

	if(qCoreApp == NULL) {
		int argc = 0;
		qCoreApp = new QCoreApplication(argc, NULL);
		qCoreApp->setApplicationName(purple_kwallet_get_ui_name());
	}

	if(!KWallet::Wallet::isEnabled()) {
		g_set_error(error, PURPLE_KWALLET_DOMAIN, 0,
		            "KWallet service is disabled.");

		return FALSE;
	}

	manager = purple_credential_manager_get_default();

	instance = purple_kwallet_provider_new();

	return purple_credential_manager_register(manager, instance, error);
}

static gboolean
kwallet_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
               G_GNUC_UNUSED gboolean shutdown,
               GError **error)
{
	PurpleCredentialManager *manager = NULL;
	gboolean ret = FALSE;

	manager = purple_credential_manager_get_default();
	ret = purple_credential_manager_unregister(manager, instance, error);

	if(!ret) {
		return ret;
	}

	if(qCoreApp != NULL) {
		delete qCoreApp;
		qCoreApp = NULL;
	}

	g_clear_object(&instance);

	return TRUE;
}

G_BEGIN_DECLS
GPLUGIN_NATIVE_PLUGIN_DECLARE(kwallet)
G_END_DECLS
