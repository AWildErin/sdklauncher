#pragma once

#include "mainview.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QTextEdit>
namespace ui
{

	class CEditConfig : public QDialog
	{
		static QJsonArray commandLineParser( const QString &argList );
	public:
		CEditConfig( CMainView *parent );
		QListWidget *m_pEditList;
		QPushButton *m_pApplyButton;
		QPushButton *m_pAddCurrentButton;
	private:
		enum Quotations
		{
			isNone = 0,
			isSingleQuote,
			isDoubleQuote
		};
	};

	class CEditConfigPopup : public QDialog
	{
		bool m_ApplyChanges = false;

	public:
		CEditConfigPopup( CEditConfig *parent );
		bool shouldApplyChanges() const;
		QComboBox *m_pTypeComboBox;
		QLineEdit *m_pNameLineEdit;
		QLineEdit *m_pUrlLineEdit;
		QLineEdit *m_pIconPathLineEdit;
		QTextEdit *m_pArgumentsListTextEdit;
		QPushButton *m_pApplyButton;
	};
} // namespace ui