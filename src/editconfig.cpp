#include "editconfig.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSizePolicy>

using namespace ui;

CEditConfig::CEditConfig( CMainView *parent ) :
	QDialog( parent )
{
	
	setWindowTitle( "Edit Launcher Configuration" );

	auto pEditConfigurationLayout = new QGridLayout( this );

	m_pEditList = new QListWidget( this );
	m_pEditList->setItemAlignment( Qt::AlignCenter );
	
	int listChildCount = parent->m_pListWidget->count();
	for ( int i = 0; i < listChildCount; i++ )
	{
		auto pMainWindowListItem = parent->m_pListWidget->item(i);
		if ( pMainWindowListItem->type() ==  CMainView::Category )
		{
			auto pListWidgetItem = new QListWidgetItem();
			pListWidgetItem->setText( pMainWindowListItem->text() );

			auto listItemFont = pListWidgetItem->font();
			listItemFont.setBold( true );
			pListWidgetItem->setFont( listItemFont );

			auto listItemContentObject = QJsonObject();
			listItemContentObject["name"] = pMainWindowListItem->text();
			listItemContentObject["urlType"] = "category";
			pListWidgetItem->setData( Qt::UserRole, listItemContentObject );

			m_pEditList->addItem( pListWidgetItem );
		}

		if ( pMainWindowListItem->type() ==  CMainView::Item )
		{
			auto pListWidgetItem = new QListWidgetItem();
			pListWidgetItem->setText( pMainWindowListItem->text() );
			pListWidgetItem->setIcon( pMainWindowListItem->icon() );
			pListWidgetItem->setData( Qt::UserRole, pMainWindowListItem->data( Qt::UserRole ) );

			m_pEditList->addItem( pListWidgetItem );
		}

	}
	
	pEditConfigurationLayout->setAlignment( Qt::AlignTop );

	pEditConfigurationLayout->addWidget( m_pEditList, 0, 0, 5, 1 );
	m_pAddCurrentButton = new QPushButton( this );
	m_pAddCurrentButton->setIcon( QIcon( ":/resource/add.png" ) );

	pEditConfigurationLayout->addWidget( m_pAddCurrentButton, 0, 1, Qt::AlignTop | Qt::AlignRight );

	auto pShiftUpButton = new QPushButton( this );
	pShiftUpButton->setIcon( QIcon( ":/resource/arrowup.png" ) );

	pEditConfigurationLayout->addWidget( pShiftUpButton, 1, 1, Qt::AlignTop | Qt::AlignRight );

	auto pEditButton = new QPushButton( this );
	pEditButton->setIcon( QIcon( ":/resource/edit.png" ) );

	pEditConfigurationLayout->addWidget( pEditButton, 2, 1, Qt::AlignTop | Qt::AlignRight );

	auto pShiftDownButton = new QPushButton( this );
	pShiftDownButton->setIcon( QIcon( ":/resource/arrowdown.png" ) );

	pEditConfigurationLayout->addWidget( pShiftDownButton, 3, 1, Qt::AlignTop | Qt::AlignRight );

	auto pRemoveCurrentButton = new QPushButton( this );
	pRemoveCurrentButton->setIcon( QIcon( ":/resource/remove.png" ) );

	pEditConfigurationLayout->addWidget( pRemoveCurrentButton, 4, 1, Qt::AlignTop | Qt::AlignRight );
	
	auto pDialogButtons = new QDialogButtonBox( this );
	m_pApplyButton = pDialogButtons->addButton( "Apply", QDialogButtonBox::ButtonRole::ApplyRole );
	pDialogButtons->addButton( "Cancel", QDialogButtonBox::ButtonRole::RejectRole );

	pEditConfigurationLayout->addWidget( pDialogButtons, 5, 0, 1, 2, Qt::AlignTop | Qt::AlignLeft );
	
	auto onCurrentRowChangedCallback = [&]( int row )
	{
		auto current = m_pEditList->item( 0 );
		if ( !current )
		{
			m_pApplyButton->setDisabled( true );
			m_pApplyButton->setToolTip( "The top element must be a category." );
			return;
		}
		m_pApplyButton->setDisabled( false );
		m_pApplyButton->setToolTip( "" );
		if ( current->data( Qt::UserRole ).toJsonObject()["urlType"] != "category" )
		{
			m_pApplyButton->setDisabled( true );
			m_pApplyButton->setToolTip( "The top element must be a category." );
		}
	};
	
	auto onAddItemButtonPressedCallback = [&]
	{

		auto pEditConfigPopup = new CEditConfigPopup( this );
		pEditConfigPopup->exec();

		if ( !pEditConfigPopup->shouldApplyChanges() )
			return;
		
		auto pNewListItem = new QListWidgetItem();
		pNewListItem->setText( pEditConfigPopup->m_pNameLineEdit->text() );
		pNewListItem->setIcon( QIcon() );

		if ( QFileInfo::exists( pEditConfigPopup->m_pIconPathLineEdit->text() ) )
			pNewListItem->setIcon( QIcon( pEditConfigPopup->m_pIconPathLineEdit->text() ) );

		auto pCurrentItem = m_pEditList->currentItem();

		auto listItemJSONContents = QJsonObject();
		listItemJSONContents["name"] = pEditConfigPopup->m_pNameLineEdit->text();

		if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 2 )
		{
			
			auto listItemFont = pNewListItem->font();
			listItemFont.setBold( true );
			pNewListItem->setFont( listItemFont );
			listItemJSONContents["urlType"] = "category";
			pNewListItem->setData( Qt::UserRole, listItemJSONContents );
			m_pEditList->currentRowChanged( 0 );
			int currentRow = m_pEditList->row( pCurrentItem );
			m_pEditList->insertItem( currentRow + 1, pNewListItem );
			return;
		}

		listItemJSONContents["url"] = pEditConfigPopup->m_pUrlLineEdit->text();
		listItemJSONContents["icon"] = pEditConfigPopup->m_pIconPathLineEdit->text();
		listItemJSONContents["urlType"] = "url";

		if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 0 )
		{
			auto jsonArgumentList = commandLineParser(pEditConfigPopup->m_pArgumentsListTextEdit->toPlainText());
			qInfo() << jsonArgumentList;
			listItemJSONContents["urlType"] = "process";
			listItemJSONContents["args"] = jsonArgumentList;
		}
		
		pNewListItem->setData( Qt::UserRole, listItemJSONContents );
		m_pEditList->currentRowChanged( 0 );

		int currentRow = pCurrentItem ? m_pEditList->row( pCurrentItem ) : 0;
		m_pEditList->insertItem( currentRow + 1, pNewListItem );

	};
	
	auto onShiftItemDownButtonPressedCallback = [&]
	{
		auto item = m_pEditList->currentItem();
		if ( item )
		{
			if ( m_pEditList->row( item ) + 1 > m_pEditList->count() - 1 )
				return;
			int currentRow = m_pEditList->row( item );
			m_pEditList->takeItem( currentRow );
			m_pEditList->insertItem( currentRow + 1, item );
			m_pEditList->setCurrentRow( currentRow + 1 );
		}
	};
	
	auto onEditItemButtonPressedCallback = [&]
	{

		auto pSelectedItem = m_pEditList->currentItem();
		if ( !pSelectedItem )
			return;
		auto listItemJSONContents = pSelectedItem->data( Qt::UserRole ).toJsonObject();
		auto pEditConfigPopup = new CEditConfigPopup( this );
		pEditConfigPopup->setWindowTitle( "Edit " + pSelectedItem->text() );
		if ( listItemJSONContents["urlType"] == "category" )
		{
			pEditConfigPopup->m_pNameLineEdit->setText( pSelectedItem->text() );
			pEditConfigPopup->m_pTypeComboBox->setCurrentIndex( 2 );
		}
		else
		{
			pEditConfigPopup->m_pNameLineEdit->setText( listItemJSONContents["name"].toString() );
			pEditConfigPopup->m_pUrlLineEdit->setText( listItemJSONContents["url"].toString() );
			pEditConfigPopup->m_pIconPathLineEdit->setText( listItemJSONContents["icon"].toString() );
			pEditConfigPopup->m_pTypeComboBox->setCurrentIndex( 1 );
			if ( listItemJSONContents["urlType"] == "process" )
			{
				pEditConfigPopup->m_pTypeComboBox->setCurrentIndex( 0 );
				auto oldJSONArgumentList = QString( "" );
				
				
				
				foreach( QJsonValue str, listItemJSONContents["args"].toArray() )
				{
					oldJSONArgumentList.append( str.toString() + " " );
				}
				pEditConfigPopup->m_pArgumentsListTextEdit->setText( oldJSONArgumentList );
			}
		}

		pEditConfigPopup->exec();
		if ( !pEditConfigPopup->shouldApplyChanges() )
			return;

		pSelectedItem->setText( pEditConfigPopup->m_pNameLineEdit->text() );
		pSelectedItem->setIcon( QIcon() );
		if ( QFileInfo::exists( pEditConfigPopup->m_pIconPathLineEdit->text() ) )
			pSelectedItem->setIcon( QIcon( pEditConfigPopup->m_pIconPathLineEdit->text() ) );

		auto newListItemJSONContents = QJsonObject();
		newListItemJSONContents["name"] = pEditConfigPopup->m_pNameLineEdit->text();
		if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 2 )
		{
			auto listItemFont = pSelectedItem->font();
			listItemFont.setBold( true );
			pSelectedItem->setFont( listItemFont );
			newListItemJSONContents["urlType"] = "category";
			pSelectedItem->setData( Qt::UserRole, newListItemJSONContents );
			pSelectedItem->setIcon( QIcon() );
			m_pEditList->currentRowChanged( 0 );
			return;
		}
		newListItemJSONContents["url"] = pEditConfigPopup->m_pUrlLineEdit->text();
		newListItemJSONContents["icon"] = pEditConfigPopup->m_pIconPathLineEdit->text();
		newListItemJSONContents["urlType"] = "url";
		if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 0 )
		{
			auto newJSONArgumentList = commandLineParser(pEditConfigPopup->m_pArgumentsListTextEdit->toPlainText());
			qInfo() << newJSONArgumentList;
			newListItemJSONContents["urlType"] = "process";
			newListItemJSONContents["args"] = newJSONArgumentList;
		}
		pSelectedItem->setData( Qt::UserRole, newListItemJSONContents );
		m_pEditList->currentRowChanged( 0 );
	};
	
	auto onShiftItemUpButtonPressedCallback = [&]
	{
		auto item = m_pEditList->currentItem();
		if ( item )
		{
			if ( m_pEditList->row( item ) - 1 < 0 )
				return;
			int currentRow = m_pEditList->row( item );
			m_pEditList->takeItem( currentRow );
			m_pEditList->insertItem( currentRow - 1, item );
			m_pEditList->setCurrentRow( currentRow - 1 );
		}
	};
	
	auto onRemoveItemButtonPressedCallback = [&]
	{
		auto item = m_pEditList->currentItem();
		if ( !item )
			return;
		int currentItemRow = m_pEditList->row( item );
		m_pEditList->takeItem( currentItemRow );
		delete item;
		m_pEditList->currentRowChanged( 0 );
	};
	
	auto onDialogButtonsPressedCallback = [&]( QAbstractButton *button )
	{
		if ( button->text() == "Apply" )
		{
			auto pParentWidget = dynamic_cast<CMainView *>( this->parentWidget() );
			auto pMainListLayout = pParentWidget->m_pListWidget;

			while ( pMainListLayout->count() > 0 )
			{
				auto pMainListLayoutItem = pMainListLayout->item( 0 );
				auto pMainListLayoutItemWidget = pMainListLayout->itemWidget(pMainListLayoutItem);
				delete pMainListLayoutItemWidget;
				delete pMainListLayoutItem;
			}

			QJsonArray *pCurrentCategory;
			auto jsonArrayVector = QVector<QPair<QString, QJsonArray *>>();

			for ( int i = 0; i < m_pEditList->count(); i++ )
			{
				auto pListItem = m_pEditList->item( i );
				auto itemJSONContents = pListItem->data( Qt::UserRole ).toJsonObject();
				if ( itemJSONContents["urlType"] == "category" )
				{
					pCurrentCategory = new QJsonArray();
					jsonArrayVector.append( QPair<QString, QJsonArray *>( itemJSONContents["name"].toString(), pCurrentCategory ) );
					continue;
				}

				pCurrentCategory->append( itemJSONContents );

			}

			auto configJSONDocument = QJsonDocument();
			auto mainJSONArray = QJsonArray();

			for ( const auto &jsonArrayStringPair : jsonArrayVector )
			{
				auto headerObject = QJsonObject();
				headerObject["header"] = jsonArrayStringPair.first;
				headerObject["content"] = *jsonArrayStringPair.second;
				mainJSONArray.append( headerObject );

				delete jsonArrayStringPair.second;
			}
			configJSONDocument.setArray( mainJSONArray );

			auto configFile = QFile( "./config.json" );
			configFile.open( QFile::WriteOnly );
			configFile.write( configJSONDocument.toJson() );
			configFile.close();

			pParentWidget->PopulateListWidget(configJSONDocument);
		}
		this->close();
	};
	
	connect( m_pEditList, &QListWidget::currentRowChanged, this, onCurrentRowChangedCallback );

	connect( m_pAddCurrentButton, &QPushButton::pressed, this, onAddItemButtonPressedCallback );

	connect( pShiftDownButton, &QPushButton::pressed, this, onShiftItemDownButtonPressedCallback );

	connect( pEditButton, &QPushButton::pressed, this, onEditItemButtonPressedCallback );

	connect( pShiftUpButton, &QPushButton::pressed, this, onShiftItemUpButtonPressedCallback );

	connect( pRemoveCurrentButton, &QPushButton::pressed, this, onRemoveItemButtonPressedCallback );

	connect( pDialogButtons, &QDialogButtonBox::clicked, this, onDialogButtonsPressedCallback );

	pEditConfigurationLayout->setRowStretch( 4, 1 );

	
	this->setFocus( Qt::NoFocusReason );
}


QJsonArray CEditConfig::commandLineParser(const QString& argList)
{
	auto newJSONArgumentList = QJsonArray();
	auto stringConstructor = QString();

	Quotations quotationState = isNone;
	bool isEscaped = false;

	foreach( auto arg, argList.split( "" ) )
	{
		if( quotationState != isNone || ( quotationState == isNone && arg != " "))
			stringConstructor.append(arg);

		if(!isEscaped && arg != R"(\)")
		{
			isEscaped = true;
			continue;
		}

		if( quotationState == isNone && arg == "'")
		{
			quotationState = isSingleQuote;
			continue;
		}

		if( quotationState == isNone && arg == R"(")")
		{
			quotationState = isDoubleQuote;
			continue;
		}

		if( quotationState == isNone && arg == " "){
			if(stringConstructor.isEmpty())
				continue;
			newJSONArgumentList.append( stringConstructor );
			stringConstructor.clear();
			continue;
		}

		if( quotationState == isSingleQuote && arg == "'")
		{
			newJSONArgumentList.append( stringConstructor );
			stringConstructor.clear();
			quotationState = isNone;
			continue;
		}

		if( quotationState == isDoubleQuote && arg == R"(")")
		{
			newJSONArgumentList.append( stringConstructor );
			stringConstructor.clear();
			quotationState = isNone;
			continue;
		}
		isEscaped = false;
	}
	if(!stringConstructor.isEmpty())
		newJSONArgumentList.append( stringConstructor );

	return newJSONArgumentList;
}

CEditConfigPopup::CEditConfigPopup( CEditConfig *parent ) :
	QDialog( parent )
{
	auto pConfigPopupLayout = new QGridLayout( this );
	pConfigPopupLayout->setAlignment( Qt::AlignTop | Qt::AlignRight );

	auto pNameLabel = new QLabel( "Name:", this );

	pConfigPopupLayout->addWidget( pNameLabel, 0, 0 );

	m_pNameLineEdit = new QLineEdit( this );

	pConfigPopupLayout->addWidget( m_pNameLineEdit, 0, 1 );

	auto pTypeLabel = new QLabel( "Type:", this );

	pConfigPopupLayout->addWidget( pTypeLabel, 0, 2 );

	m_pTypeComboBox = new QComboBox( this );
	m_pTypeComboBox->addItem( "Executable" );
	m_pTypeComboBox->addItem( "Url" );
	m_pTypeComboBox->addItem( "Category" );

	pConfigPopupLayout->addWidget( m_pTypeComboBox, 0, 3 );

	auto pUrlLabel = new QLabel( "Exec:", this );

	pConfigPopupLayout->addWidget( pUrlLabel, 1, 0 );

	m_pUrlLineEdit = new QLineEdit( this );

	pConfigPopupLayout->addWidget( m_pUrlLineEdit, 1, 1 );

	auto pIconLabel = new QLabel( "Icon:", this );

	pConfigPopupLayout->addWidget( pIconLabel, 1, 2 );

	m_pIconPathLineEdit = new QLineEdit( this );

	pConfigPopupLayout->addWidget( m_pIconPathLineEdit, 1, 3 );

	auto pArgumentsLabel = new QLabel( "Args:", this );

	pConfigPopupLayout->addWidget( pArgumentsLabel, 2, 0 );

	m_pArgumentsListTextEdit = new QTextEdit( this );
	m_pArgumentsListTextEdit->setFixedHeight( m_pIconPathLineEdit->sizeHint().height() );

	pConfigPopupLayout->addWidget( m_pArgumentsListTextEdit, 2, 1, 1, 3 );

	auto pAcceptCancelButtonBox = new QDialogButtonBox( this );
	m_pApplyButton = pAcceptCancelButtonBox->addButton( "Apply", QDialogButtonBox::ButtonRole::ApplyRole );
	pAcceptCancelButtonBox->addButton( "Cancel", QDialogButtonBox::ButtonRole::RejectRole );

	pConfigPopupLayout->addWidget( pAcceptCancelButtonBox, 3, 0, 1, 4, Qt::AlignLeft );
	
	connect( m_pNameLineEdit, &QLineEdit::textChanged, this, [&]( const QString &text )
			 {
				 if ( m_pTypeComboBox->currentIndex() == 2 )
				 {
					 m_pApplyButton->setEnabled( text.count( " " ) != text.length() );
					 m_pApplyButton->setToolTip( m_pNameLineEdit->text().count( " " ) != m_pNameLineEdit->text().length() ? "" : "Categories MUST have a name." );
				 }
			 } );

	connect( pAcceptCancelButtonBox, &QDialogButtonBox::clicked, this, [&]( QAbstractButton *button )
			 {
				 if ( button->text() == "Apply" )
				 {
					 this->m_ApplyChanges = true;
				 }
				 this->close();
			 } );

	connect( m_pTypeComboBox, &QComboBox::currentTextChanged, this, [&, pUrlLabel, pIconLabel, pArgumentsLabel]( const QString &text )
			 {
				 if ( text == "Executable" )
				 {
					 pUrlLabel->setText( "Exec:" );
					 pUrlLabel->setEnabled( true );
					 m_pUrlLineEdit->setEnabled( true );
					 pIconLabel->setEnabled( true );
					 m_pIconPathLineEdit->setEnabled( true );
					 pArgumentsLabel->setEnabled( true );
					 m_pArgumentsListTextEdit->setEnabled( true );
					 m_pApplyButton->setEnabled( true );
					 m_pApplyButton->setToolTip( "" );
					 return;
				 }
				 if ( text == "Url" )
				 {
					 pUrlLabel->setText( "Url:" );
					 pUrlLabel->setEnabled( true );
					 m_pUrlLineEdit->setEnabled( true );
					 pIconLabel->setEnabled( true );
					 m_pIconPathLineEdit->setEnabled( true );
					 pArgumentsLabel->setEnabled( false );
					 m_pArgumentsListTextEdit->setEnabled( false );
					 m_pApplyButton->setEnabled( true );
					 m_pApplyButton->setToolTip( "" );
					 return;
				 }
				 if ( text == "Category" )
				 {
					 pUrlLabel->setEnabled( false );
					 m_pUrlLineEdit->setEnabled( false );
					 pIconLabel->setEnabled( false );
					 m_pIconPathLineEdit->setEnabled( false );
					 pArgumentsLabel->setEnabled( false );
					 m_pArgumentsListTextEdit->setEnabled( false );
					 m_pApplyButton->setToolTip( m_pNameLineEdit->text().count( " " ) != m_pNameLineEdit->text().length() ? "" : "Categories MUST have a name." );
					 m_pApplyButton->setEnabled( m_pNameLineEdit->text().count( " " ) != m_pNameLineEdit->text().length() );
					 return;
				 }
			 } );
}

bool CEditConfigPopup::shouldApplyChanges() const
{
	return this->m_ApplyChanges;
}
