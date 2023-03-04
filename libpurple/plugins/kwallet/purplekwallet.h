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

#include <purple.h>

#include <kwallet.h>
#include <QQueue>

#define PURPLE_KWALLET_TYPE_PROVIDER (purple_kwallet_provider_get_type())
G_DECLARE_FINAL_TYPE(PurpleKWalletProvider, purple_kwallet_provider,
                     PURPLE_KWALLET, PROVIDER, PurpleCredentialProvider)

namespace PurpleKWalletPlugin {

class Request {
public:
	Request(QString key, GTask *task);
	virtual ~Request(void);
	virtual void execute(KWallet::Wallet *wallet) = 0;
	virtual void cancel(QString reason) = 0;
protected:
	QString key;
	GTask *task;
};

class ReadRequest : public Request {
public:
	ReadRequest(QString key, GTask *task);
	void execute(KWallet::Wallet *wallet);
	void cancel(QString reason);
};

class WriteRequest : public Request {
public:
	WriteRequest(QString key, GTask *task, QString password);
	void execute(KWallet::Wallet *wallet);
	void cancel(QString reason);
private:
	QString password;
};

class ClearRequest : public Request {
public:
	ClearRequest(QString key, GTask *task);
	void execute(KWallet::Wallet *wallet);
	void cancel(QString reason);
};

class Engine : public QObject {
	Q_OBJECT

public:
	Engine(void);
	~Engine(void);

	void open(void);
	void close(void);
	void enqueue(Request *request);
private slots:
	void opened(bool opened);
	void closed(void);
private:
	void processQueue(void);

	bool connected;
	bool externallyClosed;
	bool failed;

	KWallet::Wallet *wallet;

	QQueue<Request *> queue;
};

}
