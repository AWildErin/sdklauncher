#include "mainview.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	auto pLayout = new QVBoxLayout( this );
	pLayout->setObjectName( "SDKLayout" );

	QFile config( "./config.json" );
	config.open( QFile::ReadOnly );
	QJsonDocument JSONConfigDocument = QJsonDocument::fromJson( config.readAll() );
	config.close();
	QJsonObject JSONConfig = JSONConfigDocument.object();

	for ( auto it = JSONConfig.begin(); it != JSONConfig.end(); it++ )
	{
		// MediaSection_t section = Sections.at( it.key() );

		auto pHeader = new QLabel( it.key(), this );
		pHeader->setObjectName( "Header" );
		pLayout->addWidget( pHeader );
		QJsonArray arr = it.value().toArray();
		for ( int i = 0; i < arr.size(); i++ )
		{
			auto item = arr.at( i ).toObject();

			QPushButton *pButton = new QPushButton( this );
			pButton->setIcon( QIcon( item["icon"].toString() ) );
			pButton->setText( item["name"].toString() );
			pButton->setObjectName( "MediaItem" );

			pLayout->addWidget( pButton );
			connect( pButton, &QPushButton::pressed, this,
					 [=]()
					 {
						 auto arr = item["args"].toArray().toVariantList();
						 QStringList args;

						 foreach( QVariant vItem, arr )
						 {
							 args << vItem.toString();
						 }

						 if ( item["urlType"].toString() == "url" )
							 OpenUrl( item["url"].toString() );
						 else if ( item["urlType"].toString() == "process" )
							 OpenProcess( item["url"].toString(), args );
						 else
							 qDebug() << "Unknown URL Type: " << item["urlType"].toString();
					 } );
		}
	}

	// Set focus so we don't have focus directly on the top most button
	this->setFocus( Qt::NoFocusReason );

	this->setFixedWidth( 250 );
	this->setFixedHeight( this->sizeHint().height() );
}

void CMainView::OpenUrl( QString url )
{
	QDesktopServices::openUrl( QUrl( url ) );
}

void CMainView::OpenProcess( QString execName, QStringList params )
{
	auto pProcess = new QProcess( this );
	pProcess->setArguments( params );
	pProcess->setProgram( execName );
	pProcess->start();
}
