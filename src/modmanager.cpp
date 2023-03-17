#include "modmanager.h"

#ifdef _WIN32
#define OS_BIN_SUFFIX ".bat"
#else
#define OS_BIN_SUFFIX ".sh"
#endif

#include "editconfig.h"
#include "filedownloader.h"
#include "ziphandler.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

using namespace ui;

CModManager::CModManager( CMainView *parent ) :
	QDialog( parent )
{
	setWindowTitle( "New P2:CE Source Mod" );

	auto pManagerGridLayout = new QGridLayout( this );

	auto pModNameLabel = new QLabel( this );
	pModNameLabel->setText( "Mod Name:" );
	pManagerGridLayout->addWidget( pModNameLabel, 0, 0 );

	m_pModNameLineEdit = new QLineEdit( this );
	pManagerGridLayout->addWidget( m_pModNameLineEdit, 0, 1, 1, 3 );

	auto pModPathLabel = new QLabel( this );
	pModPathLabel->setText( "Mod Path:" );
	pManagerGridLayout->addWidget( pModPathLabel, 1, 0 );

	auto pDirectoryPushButton = new QPushButton( this );
	pDirectoryPushButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pDirectoryPushButton->setFixedWidth( pDirectoryPushButton->height() );
	pManagerGridLayout->addWidget( pDirectoryPushButton, 1, 1 );

	m_pModPathLineEdit = new QLineEdit( this );
	m_pModPathLineEdit->setText( parent->GetInstallDir() );
	m_pModPathLineEdit->setDisabled( true );
	pManagerGridLayout->addWidget( m_pModPathLineEdit, 1, 2 );

	auto pCreateCancelButtonBox = new QDialogButtonBox( this );
	m_pCreateButton = pCreateCancelButtonBox->addButton( "Create", QDialogButtonBox::AcceptRole );
	m_pCreateButton->setDisabled( true );
	m_pCreateButton->setToolTip( "Mods MUST have a name." );
	pCreateCancelButtonBox->addButton( "Cancel", QDialogButtonBox::RejectRole );

	connect( pDirectoryPushButton, &QPushButton::pressed, [this]
			 {
				 QString directoryResult = QFileDialog::getExistingDirectory( this );
				 if ( !directoryResult.isEmpty() )
					 m_pModPathLineEdit->setText( directoryResult + "/" );
			 } );

	connect( pCreateCancelButtonBox, &QDialogButtonBox::clicked, this, [this]( QAbstractButton *pCreateOrCancelButton )
			 {
				 if ( pCreateOrCancelButton->text() == "Create" )
					 this->modCreationHandler();
				 else
					 this->close();
			 } );

	connect( m_pModNameLineEdit, &QLineEdit::textChanged, this, [this]( const QString &text )
			 {
				 bool isNotEmpty = text.count( " " ) != text.count();
				 m_pCreateButton->setEnabled( isNotEmpty );
				 m_pCreateButton->setToolTip( isNotEmpty ? "" : "Mods MUST have a name." );
			 } );

	connect( m_pModPathLineEdit, &QLineEdit::textChanged, this, [this, pDirectoryPushButton]( const QString &text )
			 {
				 QFileInfo currentSelectedDirectory = QFileInfo( text );
				 bool fileExistsAndWritable = currentSelectedDirectory.exists() && currentSelectedDirectory.isDir() && currentSelectedDirectory.isWritable();
				 m_pCreateButton->setEnabled( fileExistsAndWritable );
				 m_pCreateButton->setToolTip( fileExistsAndWritable ? "" : "The folder MUST exist and application must have write access." );
				 pDirectoryPushButton->setIcon( QIcon( fileExistsAndWritable ? ":/resource/edit.png" : ":/resource/edit_error.png" ) );
			 } );

	pManagerGridLayout->addWidget( pCreateCancelButtonBox, 2, 0, 1, 3, Qt::AlignLeft );
}

bool CModManager::modZipHandler( const QFileInfo &zFile, const QString &modName, const QString &modPath )
{
	CZipHandler::Result zipper_ret;

	auto zipHandler = CZipHandler( zFile.absoluteFilePath() );

	if ( !zipHandler.IsValid() )
		return false;

	QString firstDir = QString();
	do
	{
		zipper_ret = CZipHandler::Result::ZIPPER_RESULT_SUCCESS;
		QString zfilename = zipHandler.GetFilename( nullptr );
		if ( zfilename.isEmpty() )
			return false;

		if ( zipHandler.IsDir() )
		{
			if ( firstDir.isEmpty() )
				firstDir.append( zfilename );
			QString newDir = QString( zfilename ).replace( firstDir, modName + "/" );
			QDir( modPath ).mkpath( newDir );
			++zipHandler;
			continue;
		}

		QString fixedFilePath = QString( zfilename ).replace( firstDir, modName + "/" );

		QByteArray buf = QByteArray();
		zipper_ret = zipHandler.Read( buf );
		if ( zipper_ret == CZipHandler::Result::ZIPPER_RESULT_ERROR )
		{
			break;
		}

		QFile file = QFile( modPath + fixedFilePath );
		if ( !file.open( QFile::WriteOnly ) )
		{
			return false;
		}
		file.write( buf );
		file.close();

	} while ( zipper_ret == CZipHandler::Result::ZIPPER_RESULT_SUCCESS );

	if ( zipper_ret == CZipHandler::Result::ZIPPER_RESULT_ERROR )
	{
		printf( "failed to read file\n" );
		return false;
	}

	return true;
}

void CModManager::modCreationHandler()
{
	const QString modName = m_pModNameLineEdit->text();
	const QString modPath = m_pModPathLineEdit->text();

	const QFileInfo modPathInfo = QFileInfo( modPath );
	const QFileInfo modPathFullInfo = QFileInfo( modPath + modName );

	if ( modPathFullInfo.exists() )
	{
		QMessageBox::warning( this, "Already Exists!", "This folder already exists!" );
		return;
	}

	if ( !modPathInfo.isWritable() )
	{
		QMessageBox::critical( this, "ACCESS ERROR!!", "Cannot write to " + modPathInfo.absoluteFilePath() );
		return;
	}

	if ( !QFileInfo( QDir::tempPath() ).isWritable() )
	{
		QMessageBox::critical( this, "ACCESS ERROR!!", "Cannot write to " + QDir::tempPath() );
		return;
	}
	auto pTemporaryFile = new QFile( QDir::tempPath() + "/p2ce-mod-template.zip", this );

	auto pFileDownloader = new CFileDownloader( QUrl( GIT_URL ), this );

	connect( pFileDownloader, &CFileDownloader::downloaded, this, [pFileDownloader, pTemporaryFile]
			 {
				 pTemporaryFile->open( QFile::ReadWrite );
				 pTemporaryFile->write( pFileDownloader->downloadedData() );
				 pTemporaryFile->close();
			 } );

	connect( pTemporaryFile, &QFile::aboutToClose, [this]
			 {
				 qInfo() << "UNZIPPING!";
				 if ( !modZipHandler( QFileInfo( QDir::tempPath() + "/p2ce-mod-template.zip" ), m_pModNameLineEdit->text(), m_pModPathLineEdit->text() ) )
				 {
					 QMessageBox::critical( this, "UNZIP ERROR", "Cannot unzip mod template." );
					 return;
				 }
				 QFile::remove( QDir::tempPath() + "/p2ce-mod-template.zip" );
				 this->hide();
				 auto mainParent = dynamic_cast<CMainView *>( this->parentWidget() );
				 auto editCFG = new CEditConfig( mainParent );

				 auto pCurrentItem = editCFG->m_pEditList->currentItem();
				 auto pEditConfigPopup = new CEditConfigPopup( editCFG );
				 const QString modName = m_pModNameLineEdit->text();
				 const QString modPath = m_pModPathLineEdit->text();
				 pEditConfigPopup->setWindowTitle( modName + " Config Entry" );
				 pEditConfigPopup->m_pTypeComboBox->setCurrentIndex( 0 );
				 pEditConfigPopup->m_pNameLineEdit->setText( modName );
				 pEditConfigPopup->m_pUrlLineEdit->setText( mainParent->GetInstallDir() + "p2ce" + OS_BIN_SUFFIX );
				 pEditConfigPopup->m_pArgumentsListTextEdit->append( "-game" );
				 pEditConfigPopup->m_pArgumentsListTextEdit->append( "'" + modPath + modName + "'" );
				 pEditConfigPopup->exec();

				 if ( pEditConfigPopup->shouldApplyChanges() )
				 {
					 auto pNewListItem = new QListWidgetItem();
					 pNewListItem->setText( pEditConfigPopup->m_pNameLineEdit->text() );
					 pNewListItem->setIcon( QIcon() );
					 if ( QFileInfo::exists( pEditConfigPopup->m_pIconPathLineEdit->text() ) )
						 pNewListItem->setIcon( QIcon( pEditConfigPopup->m_pIconPathLineEdit->text() ) );

					 auto listItemJSONContents = QJsonObject();
					 listItemJSONContents["name"] = pEditConfigPopup->m_pNameLineEdit->text();
					 if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 2 )
					 {
						 auto listItemFont = pNewListItem->font();
						 listItemFont.setBold( true );
						 pNewListItem->setFont( listItemFont );
						 listItemJSONContents["urlType"] = "category";
						 pNewListItem->setData( Qt::UserRole, listItemJSONContents );
						 editCFG->m_pEditList->currentRowChanged( 0 );
						 int currentRow = editCFG->m_pEditList->row( pCurrentItem );
						 editCFG->m_pEditList->insertItem( currentRow + 1, pNewListItem );
						 return;
					 }
					 listItemJSONContents["url"] = pEditConfigPopup->m_pUrlLineEdit->text();
					 listItemJSONContents["icon"] = pEditConfigPopup->m_pIconPathLineEdit->text();
					 listItemJSONContents["urlType"] = "url";
					 if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 0 )
					 {
						 auto jsonArgumentList = QJsonArray();
						 foreach( auto listArgument, pEditConfigPopup->m_pArgumentsListTextEdit->toPlainText().split( " " ) )
						 {
							 jsonArgumentList.append( listArgument );
						 }
						 listItemJSONContents["urlType"] = "process";
						 listItemJSONContents["args"] = jsonArgumentList;
					 }

					 pNewListItem->setData( Qt::UserRole, listItemJSONContents );
					 editCFG->m_pEditList->currentRowChanged( 0 );

					 int currentRow = pCurrentItem ? editCFG->m_pEditList->row( pCurrentItem ) : 0;
					 editCFG->m_pEditList->insertItem( currentRow + 1, pNewListItem );

					 editCFG->m_pApplyButton->click();
				 }
				 delete editCFG;

				 
				 auto gameInfoFile = QFile( modPath + modName + "/gameinfo.txt" );
				 
				 if ( gameInfoFile.open( QFile::ReadOnly ) )
				 {
					 QString gameInfoFileContent = QString( gameInfoFile.readAll() );
					 gameInfoFileContent = gameInfoFileContent.replace( "\"Portal 2: My Cool Template Mod\"", "\"" + modName + "\"" );
					 gameInfoFile.close();
					 if ( gameInfoFile.open( QFile::WriteOnly ) )
					 {
						 gameInfoFile.write( QByteArray( gameInfoFileContent.toStdString().c_str() ) );
						 gameInfoFile.close();
					 }
				 }

				 QMessageBox::information( this, "SUCCESS!", "Source mod successfully created!" );
				 this->close();
			 } );
}
