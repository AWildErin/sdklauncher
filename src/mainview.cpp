#include "mainview.h"

#include "FilesystemSearchProvider.h"
#include "config.h"
#include "editconfig.h"
#include "modmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	const int gameAppID = qEnvironmentVariableIntValue( "SteamAppId" );

	CFileSystemSearchProvider provider;
	if ( !provider.Available() || !provider.BIsAppInstalled( gameAppID ) )
	{
		QMessageBox::critical( this, "Missing Game!", "P2CE is not installed on this machine or is otherwise unavailable." );
		exit( EXIT_FAILURE );
	}

	char installDir[1048];
	provider.GetAppInstallDir( gameAppID, installDir, 1048 );
	m_InstallDir = QString( installDir );
	
	auto pSDKLayout = new QGridLayout( this );
	pSDKLayout->setObjectName( "SDKLayout" );
	
	m_pListWidget = new QListWidget( this );
	m_pListWidget->setSpacing(2);

	pSDKLayout->addWidget( m_pListWidget, 0, 0, 2, 1 );

	QJsonDocument JSONConfigDocument;

	if(!GetOrGenerateConfig("./config.json", JSONConfigDocument))
		QMessageBox::critical( this, "Config Error", "Couldn't load config file" );

	PopulateListWidget(JSONConfigDocument);
	
	auto pEditButton = new QPushButton( this );
	pEditButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pSDKLayout->addWidget( pEditButton, 0, 1, Qt::AlignTop );
	auto onEditConfigButtonPressedCallback = [this]
	{
		auto editConfigDialog = new CEditConfig( this );
		editConfigDialog->exec();
	};

	auto pNewModButton = new QPushButton( this );
	pNewModButton->setIcon( QIcon( ":/resource/add.png" ) );
	pSDKLayout->addWidget( pNewModButton, 1, 1, Qt::AlignTop );

	auto onNewModButtonPressedCallback = [this]
	{
		auto modManager = new CModManager( this );
		modManager->exec();
	};

	auto onElementClickedCallback = [this](QListWidgetItem* item)
	{
		auto contentObject = item->data(Qt::UserRole).toJsonObject();

		auto argumentVariantList = contentObject["args"].toArray().toVariantList();
		QStringList argumentStringList;
		foreach( QVariant vArgumentItem, argumentVariantList )
		{
			argumentStringList << vArgumentItem.toString();
		}

		if ( contentObject["urlType"].toString() == "url" )
			OpenUrl( contentObject["url"].toString().replace( "${INSTALLDIR}", m_InstallDir ) );
		else if ( contentObject["urlType"].toString() == "process" )
			OpenProcess( contentObject["url"].toString().replace( "${INSTALLDIR}", m_InstallDir ), argumentStringList.replaceInStrings( "${INSTALLDIR}", m_InstallDir ) );
		else
			qDebug() << "Unknown URL Type: " << contentObject["urlType"].toString();
	};

	pSDKLayout->setRowStretch( 1, 1 );

	connect( pEditButton, &QPushButton::pressed, this, onEditConfigButtonPressedCallback );
	connect( pNewModButton, &QPushButton::pressed, this, onNewModButtonPressedCallback );
	connect( m_pListWidget, &QListWidget::itemClicked, this, onElementClickedCallback );

	pSDKLayout->setAlignment( Qt::AlignTop );

	this->resize(280, 360);
	
	this->setFocus( Qt::NoFocusReason );
}

bool CMainView::GetOrGenerateConfig(const QString &filePath, QJsonDocument &JSONConfigDocument )
{
	QFile configFile( filePath );
	if ( !configFile.exists() && configFile.open( QFile::WriteOnly ) )
	{
		JSONConfigDocument = defaultConfig();
		configFile.write( JSONConfigDocument.toJson() );
		configFile.close();
	}
	else if ( configFile.open( QFile::ReadOnly ) )
	{

		JSONConfigDocument = QJsonDocument::fromJson( configFile.readAll() );
		configFile.close();
	}
	else
		return false;
	return true;
}

void CMainView::PopulateListWidget(const QJsonDocument &JSONConfigDocument)
{
	QJsonArray JSONConfig = JSONConfigDocument.array();
	for (const auto & it : JSONConfig)
	{
		auto HeaderObject = it.toObject();

		auto pBaseItem = new QListWidgetItem(nullptr, WidgetItemType::Category);

		pBaseItem->setText(HeaderObject["header"].toString());

		auto itemFont = pBaseItem->font();
		itemFont.setBold(true);
		pBaseItem->setFont(itemFont);

		pBaseItem->setFlags(Qt::NoItemFlags );
		pBaseItem->setForeground(Qt::white);

		auto pBaseItemWidget = new QWidget(this);

		auto pSDKListWidgetItemtLayout = new QVBoxLayout( pBaseItemWidget );

		pSDKListWidgetItemtLayout->setContentsMargins(0,18,0,0);

		pBaseItem->setSizeHint( QSize(0,20) );

		auto pHeader = new QLabel( pBaseItemWidget );

		pHeader->setObjectName( "Header" );

		pSDKListWidgetItemtLayout->addWidget( pHeader );

		QJsonArray contentArray = HeaderObject["content"].toArray();

		m_pListWidget->addItem(pBaseItem);
		m_pListWidget->setItemWidget(pBaseItem,  pBaseItemWidget );

		for ( const auto &jsonContentObject : contentArray )
		{
			auto contentObject = jsonContentObject.toObject();

			auto pListWidgetItem = new QListWidgetItem(nullptr, WidgetItemType::Item);
			pListWidgetItem->setIcon( QIcon( contentObject["icon"].toString() ) );
			pListWidgetItem->setText( contentObject["name"].toString() );
			pListWidgetItem->setData( Qt::UserRole, contentObject );

			m_pListWidget->addItem(pListWidgetItem);
		}


	}
}

void CMainView::OpenUrl( const QString &url )
{
	QDesktopServices::openUrl( QUrl( url ) );
}

void CMainView::OpenProcess( const QString &execName, const QStringList &params )
{
	qInfo() << execName;
	qInfo() << params;
	auto pProcess = new QProcess( this );
	pProcess->start( execName, params );
}
QString CMainView::GetInstallDir()
{
	return m_InstallDir;
}
