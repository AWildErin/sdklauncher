#include "filedownloader.h"

CFileDownloader::CFileDownloader( const QUrl &imageUrl, QObject *parent ) :
	QObject( parent )
{
	QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
	sslConfig.setProtocol( QSsl::TlsV1_2 );
	m_pNetworkAccessManager = new QNetworkAccessManager();

	connect( m_pNetworkAccessManager, SIGNAL( finished( QNetworkReply * ) ), this, SLOT( fileDownloaded( QNetworkReply * ) ) );

	QNetworkRequest networkRequest;
	networkRequest.setSslConfiguration( sslConfig );
	networkRequest.setUrl( imageUrl );

	m_pNetworkAccessManager->get( networkRequest );
}

CFileDownloader::~CFileDownloader() = default;

void CFileDownloader::fileDownloaded( QNetworkReply *pReply )
{
	m_DownloadedData = pReply->readAll();
	pReply->deleteLater();
	emit downloaded();
}

QByteArray CFileDownloader::downloadedData() const
{
	return m_DownloadedData;
}