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

/* Since we're using GTask to test asynchrous functions, we need to use a main
 * loop.
 */
static GMainLoop *loop = NULL;

/******************************************************************************
 * TestCredentialProviderEmpty
 *****************************************************************************/
#define TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY (test_purple_credential_provider_empty_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleCredentialProviderEmpty,
                     test_purple_credential_provider_empty,
                     TEST_PURPLE, CREDENTIAL_PROVIDER_EMPTY,
                     PurpleCredentialProvider)

struct _TestPurpleCredentialProviderEmpty {
	PurpleCredentialProvider parent;
};

G_DEFINE_TYPE(TestPurpleCredentialProviderEmpty,
              test_purple_credential_provider_empty,
              PURPLE_TYPE_CREDENTIAL_PROVIDER)

static void
test_purple_credential_provider_empty_read_password_async(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                          G_GNUC_UNUSED PurpleAccount *account,
                                                          G_GNUC_UNUSED GCancellable *cancellable,
                                                          G_GNUC_UNUSED GAsyncReadyCallback callback,
                                                          G_GNUC_UNUSED gpointer data)
{
}

static gchar *
test_purple_credential_provider_empty_read_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                           G_GNUC_UNUSED GAsyncResult *result,
                                                           G_GNUC_UNUSED GError **error)
{
	return NULL;
}

static void
test_purple_credential_provider_empty_write_password_async(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                           G_GNUC_UNUSED PurpleAccount *account,
                                                           G_GNUC_UNUSED const gchar *password,
                                                           G_GNUC_UNUSED GCancellable *cancellable,
                                                           G_GNUC_UNUSED GAsyncReadyCallback callback,
                                                           G_GNUC_UNUSED gpointer data)
{
}

static gboolean
test_purple_credential_provider_empty_write_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                            G_GNUC_UNUSED GAsyncResult *result,
                                                            G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

static void
test_purple_credential_provider_empty_class_init(G_GNUC_UNUSED TestPurpleCredentialProviderEmptyClass *klass)
{
}

static void
test_purple_credential_provider_empty_init(G_GNUC_UNUSED TestPurpleCredentialProviderEmpty *provider)
{
}

/******************************************************************************
 * purple_credential_provider_is_valid tests
 *****************************************************************************/
static void
test_purple_credential_provider_is_valid_no_id(void) {
	PurpleCredentialProvider *provider = NULL;
	GError *error = NULL;

	provider = g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY,
		"name", "name",
		NULL);

	g_assert_false(purple_credential_provider_is_valid(provider, &error));
	g_assert_error(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 0);
	g_clear_error(&error);

	g_object_unref(G_OBJECT(provider));
}

static void
test_purple_credential_provider_is_valid_no_name(void) {
	PurpleCredentialProvider *provider = NULL;
	GError *error = NULL;

	provider = g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY,
		"id", "id",
		NULL);

	g_assert_false(purple_credential_provider_is_valid(provider, &error));
	g_assert_error(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 1);
	g_clear_error(&error);

	g_object_unref(G_OBJECT(provider));
}

static void
test_purple_credential_provider_is_valid_no_reader(void) {
	PurpleCredentialProvider *provider = NULL;
	PurpleCredentialProviderClass *klass = NULL;
	GError *error = NULL;

	provider = g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY,
		"id", "id",
		"name", "name",
		NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	klass->read_password_async = NULL;
	klass->read_password_finish = NULL;
	klass->write_password_async = test_purple_credential_provider_empty_write_password_async;
	klass->write_password_finish = test_purple_credential_provider_empty_write_password_finish;

	g_assert_false(purple_credential_provider_is_valid(provider, &error));
	g_assert_error(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 2);
	g_clear_error(&error);

	g_object_unref(G_OBJECT(provider));
}

static void
test_purple_credential_provider_is_valid_no_writer(void) {
	PurpleCredentialProvider *provider = NULL;
	PurpleCredentialProviderClass *klass = NULL;
	GError *error = NULL;

	provider = g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY,
		"id", "id",
		"name", "name",
		NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	klass->read_password_async = test_purple_credential_provider_empty_read_password_async;
	klass->read_password_finish = test_purple_credential_provider_empty_read_password_finish;
	klass->write_password_async = NULL;
	klass->write_password_finish = NULL;

	g_assert_false(purple_credential_provider_is_valid(provider, &error));
	g_assert_error(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 3);
	g_clear_error(&error);

	g_object_unref(G_OBJECT(provider));
}

static void
test_purple_credential_provider_is_valid_valid(void) {
	PurpleCredentialProvider *provider = NULL;
	PurpleCredentialProviderClass *klass = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	provider = g_object_new(
		TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER_EMPTY,
		"id", "id",
		"name", "name",
		NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	klass->read_password_async = test_purple_credential_provider_empty_read_password_async;
	klass->read_password_finish = test_purple_credential_provider_empty_read_password_finish;
	klass->write_password_async = test_purple_credential_provider_empty_write_password_async;
	klass->write_password_finish = test_purple_credential_provider_empty_write_password_finish;

	ret = purple_credential_provider_is_valid(provider, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	g_object_unref(G_OBJECT(provider));
}

/******************************************************************************
 * TestPurpleCredentialProvider
 *****************************************************************************/
#define TEST_PURPLE_TYPE_CREDENTIAL_PROVIDER (test_purple_credential_provider_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleCredentialProvider,
                     test_purple_credential_provider,
                     TEST_PURPLE, CREDENTIAL_PROVIDER,
                     PurpleCredentialProvider)

struct _TestPurpleCredentialProvider {
	PurpleCredentialProvider parent;

	gboolean read_password_async;
	gboolean read_password_finish;
	gboolean write_password_async;
	gboolean write_password_finish;
	gboolean clear_password_async;
	gboolean clear_password_finish;
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
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);
	GTask *task = NULL;

	provider->read_password_async = TRUE;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_pointer(task, NULL, NULL);
	g_object_unref(G_OBJECT(task));
}

static gchar *
test_purple_credential_provider_read_password_finish(PurpleCredentialProvider *p,
                                                     G_GNUC_UNUSED GAsyncResult *result,
                                                     G_GNUC_UNUSED GError **error)
{
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	provider->read_password_finish = TRUE;

	return NULL;
}

static void
test_purple_credential_provider_write_password_async(PurpleCredentialProvider *p,
                                                     G_GNUC_UNUSED PurpleAccount *account,
                                                     G_GNUC_UNUSED const gchar *password,
                                                     GCancellable *cancellable,
                                                     GAsyncReadyCallback callback,
                                                     gpointer data)
{
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);
	GTask *task = NULL;

	provider->write_password_async = TRUE;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_boolean(task, TRUE);
	g_object_unref(G_OBJECT(task));
}

static gboolean
test_purple_credential_provider_write_password_finish(PurpleCredentialProvider *p,
                                                      G_GNUC_UNUSED GAsyncResult *result,
                                                      G_GNUC_UNUSED GError **error)
{
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	provider->write_password_finish = TRUE;

	return FALSE;
}

static void
test_purple_credential_provider_clear_password_async(PurpleCredentialProvider *p,
                                                     G_GNUC_UNUSED PurpleAccount *account,
                                                     GCancellable *cancellable,
                                                     GAsyncReadyCallback callback,
                                                     gpointer data)
{
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);
	GTask *task = NULL;

	provider->clear_password_async = TRUE;

	task = g_task_new(p, cancellable, callback, data);
	g_task_return_boolean(task, TRUE);
	g_object_unref(G_OBJECT(task));
}

static gboolean
test_purple_credential_provider_clear_password_finish(PurpleCredentialProvider *p,
                                                      G_GNUC_UNUSED GAsyncResult *result,
                                                      G_GNUC_UNUSED GError **error)
{
	TestPurpleCredentialProvider *provider = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	provider->clear_password_finish = TRUE;

	return FALSE;
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
 * TestPurpleCredentialProvider Tests
 *****************************************************************************/
static gboolean
test_purple_credential_provider_timeout_cb(gpointer data) {
	g_main_loop_quit((GMainLoop *)data);

	g_warning("timed out waiting for the callback function to be called");

	return FALSE;
}

static void
test_purple_credential_provider_test_read_cb(GObject *obj, GAsyncResult *res,
                                             gpointer d)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);
	gchar *password = NULL;

	password = purple_credential_provider_read_password_finish(provider, res,
	                                                           NULL);

	g_object_unref(G_OBJECT(account));

	g_main_loop_quit(loop);

	g_assert_null(password);
}

static gboolean
test_purple_credential_provider_test_read_idle(gpointer data) {
	PurpleCredentialProvider *p = PURPLE_CREDENTIAL_PROVIDER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");

	purple_credential_provider_read_password_async(p, account, NULL,
	                                               test_purple_credential_provider_test_read_cb,
	                                               account);

	return FALSE;
}

static void
test_purple_credential_provider_test_read(void) {
	PurpleCredentialProvider *p = test_purple_credential_provider_new();
	TestPurpleCredentialProvider *tp = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	g_idle_add(test_purple_credential_provider_test_read_idle, p);
	g_timeout_add_seconds(10, test_purple_credential_provider_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_true(tp->read_password_async);
	g_assert_true(tp->read_password_finish);
	g_assert_false(tp->write_password_async);
	g_assert_false(tp->write_password_finish);
	g_assert_false(tp->clear_password_async);
	g_assert_false(tp->clear_password_finish);

	g_object_unref(p);
}

static void
test_purple_credential_provider_test_write_cb(GObject *obj, GAsyncResult *res,
                                              gpointer d)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);

	test_purple_credential_provider_write_password_finish(provider, res, NULL);

	g_object_unref(G_OBJECT(account));

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_provider_test_write_idle(gpointer data) {
	PurpleCredentialProvider *p = PURPLE_CREDENTIAL_PROVIDER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");

	purple_credential_provider_write_password_async(p, account, NULL, NULL,
	                                                test_purple_credential_provider_test_write_cb,
	                                                account);

	return FALSE;
}

static void
test_purple_credential_provider_test_write(void) {
	PurpleCredentialProvider *p = test_purple_credential_provider_new();
	TestPurpleCredentialProvider *tp = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	g_idle_add(test_purple_credential_provider_test_write_idle, p);
	g_timeout_add_seconds(10, test_purple_credential_provider_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_false(tp->read_password_async);
	g_assert_false(tp->read_password_finish);
	g_assert_true(tp->write_password_async);
	g_assert_true(tp->write_password_finish);
	g_assert_false(tp->clear_password_async);
	g_assert_false(tp->clear_password_finish);

	g_object_unref(G_OBJECT(p));
}

static void
test_purple_credential_provider_test_clear_cb(GObject *obj, GAsyncResult *res,
                                              gpointer d)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	PurpleAccount *account = PURPLE_ACCOUNT(d);

	test_purple_credential_provider_clear_password_finish(provider, res, NULL);

	g_object_unref(G_OBJECT(account));

	g_main_loop_quit(loop);
}

static gboolean
test_purple_credential_provider_test_clear_idle(gpointer data) {
	PurpleCredentialProvider *p = PURPLE_CREDENTIAL_PROVIDER(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");

	purple_credential_provider_clear_password_async(p, account, NULL,
	                                                test_purple_credential_provider_test_clear_cb,
	                                                account);

	return FALSE;
}

static void
test_purple_credential_provider_test_clear(void) {
	PurpleCredentialProvider *p = test_purple_credential_provider_new();
	TestPurpleCredentialProvider *tp = TEST_PURPLE_CREDENTIAL_PROVIDER(p);

	g_idle_add(test_purple_credential_provider_test_clear_idle, p);
	g_timeout_add_seconds(10, test_purple_credential_provider_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_false(tp->read_password_async);
	g_assert_false(tp->read_password_finish);
	g_assert_false(tp->write_password_async);
	g_assert_false(tp->write_password_finish);
	g_assert_true(tp->clear_password_async);
	g_assert_true(tp->clear_password_finish);

	g_object_unref(p);
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

	g_test_add_func("/credential-provider/is-valid/no-id",
	                test_purple_credential_provider_is_valid_no_id);
	g_test_add_func("/credential-provider/is-valid/no-name",
	                test_purple_credential_provider_is_valid_no_name);
	g_test_add_func("/credential-provider/is-valid/no-reader",
	                test_purple_credential_provider_is_valid_no_reader);
	g_test_add_func("/credential-provider/is-valid/no-writer",
	                test_purple_credential_provider_is_valid_no_writer);
	g_test_add_func("/credential-provider/is-valid/valid",
	                test_purple_credential_provider_is_valid_valid);

	g_test_add_func("/credential-provider/functional/read",
	                test_purple_credential_provider_test_read);
	g_test_add_func("/credential-provider/functional/write",
	                test_purple_credential_provider_test_write);
	g_test_add_func("/credential-provider/functional/clear",
	                test_purple_credential_provider_test_clear);

	ret = g_test_run();

	g_main_loop_unref(loop);

	return ret;
}
