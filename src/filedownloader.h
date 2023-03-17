#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

class CFileDownloader : public QObject
{
	Q_OBJECT
public:
	explicit CFileDownloader( const QUrl &imageUrl, QObject *parent = 0 );
	virtual ~CFileDownloader();
	QByteArray downloadedData() const;

signals:
	void downloaded();

private slots:
	void fileDownloaded( QNetworkReply *pReply );

private:
	QNetworkAccessManager *m_pNetworkAccessManager;
	QByteArray m_DownloadedData;
};
