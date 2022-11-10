#include "mainview.h"

#include "config.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	auto pLayout = new QVBoxLayout( this );
	pLayout->setObjectName( "SDKLayout" );

	QFile configFile( "./config.json" );
	QJsonDocument JSONConfigDocument;
	if ( !configFile.exists() && configFile.open( QFile::WriteOnly ) )
	{
		JSONConfigDocument = defaultConfig();
		configFile.write( JSONConfigDocument.toJson() );
		configFile.close();
	}
	else if ( configFile.open (QFile::ReadOnly ) )
	{
		configFile.open( QFile::ReadOnly );
		JSONConfigDocument = QJsonDocument::fromJson( configFile.readAll() );
		configFile.close();
	}
	else
	{
		QMessageBox::critical( this, "Config Error", "Couldn't load config file" );
		return;
	}

	QJsonObject JSONConfig = JSONConfigDocument.object();

	for ( auto it = JSONConfig.begin(); it != JSONConfig.end(); it++ )
	{
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

			auto pushButton = [=]()
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
			};

			connect( pButton, &QPushButton::pressed, this, pushButton);
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
	pProcess->start( execName, params );
}
