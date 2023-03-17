#pragma once

#include "ziphandler.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSplashScreen>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>

namespace ui
{
	class CMainView : public QDialog
	{
		Q_OBJECT
	public:
		enum WidgetItemType
		{
			Category = 0,
			Item,
		};

	public:
		CMainView( QWidget *pParent );
		QListWidget *m_pListWidget;
		static void OpenUrl( const QString &url );
		void OpenProcess( const QString &execName, const QStringList &params );
		static bool GetOrGenerateConfig( const QString &filePath, QJsonDocument &JSONConfigDocument );
		void PopulateListWidget( const QJsonDocument &JSONConfigDocument );
		QString GetInstallDir();

	private:
		QString m_InstallDir;
	};
} // namespace ui