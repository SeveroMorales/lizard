/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

#include "test_ui.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

/* Since we're using GTask to test asynchronous functions, we need to use a
 * main loop.
 */
static GMainLoop *loop = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
test_purple_credential_manager_timeout_cb(gpointer data) {
	g_main_loop_quit((GMainLoop *)data);

	g_warning("timed out waiting for the callback function to be called");

	return FALSE;
}

/******************************************************************************
 * TestPurpleCredentialProvider Implementation
 *****************************************************************************/
#define TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER (test_purple_credential_provider_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleCredentialProvider,
                     test_purple_credential_provider,
                     TEST_PURPLE, CREDENTIAL_PROVIDER,
                     PurpleCredentialProvider)

struct _TestPurpleCredentialProvider {
	PurpleCredentialProvider parent;

	PurpleRequestFields *fields;
};

G_DEFINE_TYPE(TestPurpleCredentialProvider,
              test_purple_credential_provider,
              PURPLE_TYPE_CREDENTIAL_PROVIDER)

static void
test_purple_credential_provider_read_password_async(PurpleCredentialProvider *p,
                                                    G_GNUC_UNUSED PurpleAccount *account,
                                                    GCancellable *cancellable,
                                                    GAsyncReadyCallback callback,
                                                    gpointer data)
{
	GTask *task = NULL;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_pointer(task, g_strdup("password"), g_free);
	g_clear_object(&task);
}

static gchar *
test_purple_credential_provider_read_password_finish(PurpleCredentialProvider *p,
                                                     GAsyncResult *result,
                                                     GError **error)
{
	g_return_val_if_fail(g_task_is_valid(result, p), NULL);

	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
test_purple_credential_provider_write_password_async(PurpleCredentialProvider *p,
                                                     G_GNUC_UNUSED PurpleAccount *account,
                                                     G_GNUC_UNUSED const gchar *password,
                                                     GCancellable *cancellable,
                                                     GAsyncReadyCallback callback,
                                                     gpointer data)
{
	GTask *task = NULL;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_boolean(task, TRUE);
	g_clear_object(&task);
}

static gboolean
test_purple_credential_provider_write_password_finish(PurpleCredentialProvider *p,
                                                      GAsyncResult *result,
                                                      GError **error)
{
	g_return_val_if_fail(g_task_is_valid(result, p), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
test_purple_credential_provider_clear_password_async(PurpleCredentialProvider *p,
                                                     G_GNUC_UNUSED PurpleAccount *account,
                                                     GCancellable *cancellable,
                                                     GAsyncReadyCallback callback,
                                                     gpointer data)
{
	GTask *task = NULL;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_boolean(task, TRUE);
	g_clear_object(&task);
}

static gboolean
test_purple_credential_provider_clear_password_finish(PurpleCredentialProvider *p,
                                                      GAsyncResult *result,
                                                      GError **error)
{
	g_return_val_if_fail(g_task_is_valid(result, p), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
test_purple_credential_provider_init(G_GNUC_UNUSED TestPurpleCredentialProvider *provider) {
}

static void
test_purple_credential_provider_class_init(TestPurpleCredentialProviderClass *klass)
{
	PurpleCredentialProviderClass *provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);

	provider_class->read_password_async = test_purple_credential_provider_read_password_async;
	provider_class->read_password_finish = test_purple_credential_provider_read_password_finish;
	provider_class->write_password_async = test_purple_credential_provider_write_password_async;
	provider_class->write_password_finish = test_purple_credential_provider_write_password_finish;
	provider_class->clear_password_async = test_purple_credential_provider_clear_password_async;
	provider_class->clear_password_finish = test_purple_credential_provider_clear_password_finish;
}

static PurpleCredentialProvider *
test_purple_credential_provider_new(void) {
	return g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER,
		"id", "test-provider",
		"name", "Test Provider",
		NULL);
}

/******************************************************************************
 * Get Default Tests
 *****************************************************************************/
static void
test_purple_credential_manager_get_default(void) {
	PurpleCredentialManager *manager1 = NULL, *manager2 = NULL;

	manager1 = purple_credential_manager_get_default();
	g_assert_true(PURPLE_IS_CREDENTIAL_MANAGER(manager1));

	manager2 = purple_credential_manager_get_default();
	g_assert_true(PURPLE_IS_CREDENTIAL_MANAGER(manager2));

	g_assert_true(manager1 == manager2);
}

/******************************************************************************
 * Registration Tests
 *****************************************************************************/
static void
test_purple_credential_manager_registration(void) {
	PurpleCredentialManager *manager = NULL;
	PurpleCredentialProvider *provider = NULL;
	GError *error = NULL;
	gboolean r = FALSE;

	manager = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);
	g_assert_true(PURPLE_IS_CREDENTIAL_MANAGER(manager));

	provider = test_purple_credential_provider_new();

	/* Register the first time cleanly. */
	r = purple_credential_manager_register(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Register again and verify the error. */
	r = purple_credential_manager_register(manager, provider, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	/* Unregister the provider. */
	r = purple_credential_manager_unregister(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Unregister the provider again and verify the error. */
	r = purple_credential_manager_unregister(manager, provider, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	/* Final clean ups. */
	g_clear_object(&provider);
	g_clear_object(&manager);
}

/******************************************************************************
 * Set Active Tests
 *****************************************************************************/
static void
test_purple_credential_manager_set_active_null(void) {
	PurpleCredentialManager *manager = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	manager = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	ret = purple_credential_manager_set_active(manager, NULL, &error);

	g_assert_no_error(error);
	g_assert_true(ret);

	g_clear_object(&manager);
}

static void
test_purple_credential_manager_set_active_non_existent(void) {
	PurpleCredentialManager *manager = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	manager = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	ret = purple_credential_manager_set_active(manager, "foo", &error);

	g_assert_false(ret);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	g_clear_object(&manager);
}

static void
test_purple_credential_manager_set_active_normal(void) {
	PurpleCredentialManager *manager = NULL;
	PurpleCredentialProvider *provider = NULL;
	GError *error = NULL;
	gboolean r = FALSE;

	manager = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	/* Create the provider and register it in the manager. */
	provider = test_purple_credential_provider_new();
	r = purple_credential_manager_register(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Set the provider as active and verify it was successful. */
	r = purple_credential_manager_set_active(manager, "test-provider",
	                                         &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Verify that unregistering the provider fails. */
	r = purple_credential_manager_unregister(manager, provider, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	/* Now unset the active provider. */
	r = purple_credential_manager_set_active(manager, NULL, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Finally unregister the provider now that it's no longer active. */
	r = purple_credential_manager_unregister(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* And our final cleanup. */
	g_clear_object(&provider);
	g_clear_object(&manager);
}

/******************************************************************************
 * No Provider Tests
 *****************************************************************************/
static void
test_purple_credential_manager_no_provider_read_password_cb(GObject *obj,
                                                            GAsyncResult *res,
                                                            gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gchar *password = NULL;

	password = purple_credential_manager_read_password_finish(manager, res,
	                                                          &error);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);
	g_assert_null(password);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_no_provider_read_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_read_password_async(m, account, NULL,
	                                              test_purple_credential_manager_no_provider_read_password_cb,
	                                              account);

	return FALSE;
}

static void
test_purple_credential_manager_no_provider_read_password_async(void) {
	PurpleCredentialManager *m = NULL;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	g_idle_add(test_purple_credential_manager_no_provider_read_password_idle, m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	g_clear_object(&m);
}

static void
test_purple_credential_manager_no_provider_write_password_cb(GObject *obj,
                                                             GAsyncResult *res,
                                                             gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gboolean r = FALSE;

	r = purple_credential_manager_write_password_finish(manager, res, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_no_provider_write_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_write_password_async(m, account, NULL, NULL,
	                                               test_purple_credential_manager_no_provider_write_password_cb,
	                                               account);

	return FALSE;
}

static void
test_purple_credential_manager_no_provider_write_password_async(void) {
	PurpleCredentialManager *m = NULL;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	g_idle_add(test_purple_credential_manager_no_provider_write_password_idle,
	           m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	g_clear_object(&m);
}

static void
test_purple_credential_manager_no_provider_clear_password_cb(GObject *obj,
                                                             GAsyncResult *res,
                                                             gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gboolean r = FALSE;

	r = purple_credential_manager_clear_password_finish(manager, res, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_no_provider_clear_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_clear_password_async(m, account, NULL,
	                                               test_purple_credential_manager_no_provider_clear_password_cb,
	                                               account);

	return FALSE;
}

static void
test_purple_credential_manager_no_provider_clear_password_async(void) {
	PurpleCredentialManager *m = NULL;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

	g_idle_add(test_purple_credential_manager_no_provider_clear_password_idle,
	           m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	g_clear_object(&m);
}

/******************************************************************************
 * Provider Tests
 *****************************************************************************/
static void
test_purple_credential_manager_read_password_cb(GObject *obj, GAsyncResult *res,
                                                gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gchar *password = NULL;

	password = purple_credential_manager_read_password_finish(manager, res,
	                                                          &error);
	g_assert_no_error(error);
	g_assert_cmpstr(password, ==, "password");
	g_free(password);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_read_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_read_password_async(m, account, NULL,
	                                              test_purple_credential_manager_read_password_cb,
	                                              account);

	return FALSE;
}

static void
test_purple_credential_manager_read_password_async(void) {
	PurpleCredentialManager *m = NULL;
	PurpleCredentialProvider *p = NULL;
	GError *e = NULL;
	gboolean r = FALSE;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);
	p = test_purple_credential_provider_new();

	r = purple_credential_manager_register(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_set_active(m, "test-provider", &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_idle_add(test_purple_credential_manager_read_password_idle, m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	r = purple_credential_manager_set_active(m, NULL, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_unregister(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_clear_object(&p);
	g_clear_object(&m);
}

static void
test_purple_credential_manager_write_password_cb(GObject *obj,
                                                 GAsyncResult *res, gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gboolean r = FALSE;

	r = purple_credential_manager_write_password_finish(manager, res, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_write_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_write_password_async(m, account, NULL, NULL,
	                                               test_purple_credential_manager_write_password_cb,
	                                               account);

	return FALSE;
}

static void
test_purple_credential_manager_write_password_async(void) {
	PurpleCredentialManager *m = NULL;
	PurpleCredentialProvider *p = NULL;
	GError *e = NULL;
	gboolean r = FALSE;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);
	p = test_purple_credential_provider_new();

	r = purple_credential_manager_register(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_set_active(m, "test-provider", &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_idle_add(test_purple_credential_manager_write_password_idle, m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	r = purple_credential_manager_set_active(m, NULL, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_unregister(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_clear_object(&p);
	g_clear_object(&m);
}

static void
test_purple_credential_manager_clear_password_cb(GObject *obj,
                                                 GAsyncResult *res, gpointer d)
{
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	GError *error = NULL;
	gboolean r = FALSE;

	r = purple_credential_manager_clear_password_finish(manager, res, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_manager_clear_password_idle(gpointer data) {
	PurpleCredentialManager *m = PURPLE_CREDENTIAL_MANAGER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");
	purple_account_set_remember_password(account, TRUE);

	purple_credential_manager_clear_password_async(m, account, NULL,
	                                               test_purple_credential_manager_clear_password_cb,
	                                               account);

	return FALSE;
}

static void
test_purple_credential_manager_clear_password_async(void) {
	PurpleCredentialManager *m = NULL;
	PurpleCredentialProvider *p = NULL;
	GError *e = NULL;
	gboolean r = FALSE;

	m = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);
	p = test_purple_credential_provider_new();

	r = purple_credential_manager_register(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_set_active(m, "test-provider", &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_idle_add(test_purple_credential_manager_clear_password_idle, m);
	g_timeout_add_seconds(10, test_purple_credential_manager_timeout_cb, loop);

	g_main_loop_run(loop);

	r = purple_credential_manager_set_active(m, NULL, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	r = purple_credential_manager_unregister(m, p, &e);
	g_assert_no_error(e);
	g_assert_true(r);

	g_clear_object(&p);
	g_clear_object(&m);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_test_add_func("/credential-manager/get-default",
	                test_purple_credential_manager_get_default);
	g_test_add_func("/credential-manager/registration",
	                test_purple_credential_manager_registration);
	g_test_add_func("/credential-manager/set-active/null",
	                test_purple_credential_manager_set_active_null);
	g_test_add_func("/credential-manager/set-active/non-existent",
	                test_purple_credential_manager_set_active_non_existent);
	g_test_add_func("/credential-manager/set-active/normal",
	                test_purple_credential_manager_set_active_normal);

	g_test_add_func("/credential-manager/no-provider/read-password-async",
	                test_purple_credential_manager_no_provider_read_password_async);
	g_test_add_func("/credential-manager/no-provider/write-password-async",
	                test_purple_credential_manager_no_provider_write_password_async);
	g_test_add_func("/credential-manager/no-provider/clear-password-async",
	                test_purple_credential_manager_no_provider_clear_password_async);

	g_test_add_func("/credential-manager/provider/read-password-async",
	                test_purple_credential_manager_read_password_async);
	g_test_add_func("/credential-manager/provider/write-password-async",
	                test_purple_credential_manager_write_password_async);
	g_test_add_func("/credential-manager/provider/clear-password-async",
	                test_purple_credential_manager_clear_password_async);

	ret = g_test_run();

	g_main_loop_unref(loop);

	return ret;
}
