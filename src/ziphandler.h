#pragma once

#include <QString>
#include <unzip.h>
#include <zip.h>

class CZipHandler
{
private:
	static constexpr int BUF_SIZE = 8192;
	static constexpr int MAX_NAMELEN = 256;
	bool isValid = true;
	unzFile zfile;

public:
	enum class Result
	{
		ZIPPER_RESULT_ERROR = 0,
		ZIPPER_RESULT_SUCCESS,
		ZIPPER_RESULT_SUCCESS_EOF
	};

	int operator++(){
		return unzGoToNextFile( zfile );
	};

	~CZipHandler(){
		unzClose( zfile );
	}

	bool IsValid(){
		return isValid;
	}

	CZipHandler(const QString& path);
	Result Read( QByteArray &fileContents );
	bool SkipFile();
	QString GetFilename(bool *isUtf8 );
	bool IsDir();
	uint64_t GetFileSize();
};
