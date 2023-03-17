#pragma once

#include "mainview.h"

#include <QDialog>
#include <QFileInfo>
#include <QSplashScreen>

#define GIT_URL "https://codeload.github.com/craftablescience/Portal2-Example-Mod/zip/refs/tags/v1.0"

namespace ui
{
	class CModManager : public QDialog
	{
	public:
		CModManager( CMainView *pCreateOrCancelButton );

	private:
		void modCreationHandler();
		static bool modZipHandler( const QFileInfo &zFile, const QString &modName, const QString &modPath );
		QLineEdit *m_pModNameLineEdit;
		QLineEdit *m_pModPathLineEdit;
		QPushButton *m_pCreateButton;
	};
} // namespace ui
