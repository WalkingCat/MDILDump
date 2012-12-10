#pragma once
class cli_metadata_reader
{
	const std::wstring filename;
	CComPtr<IMetaDataImport2> metadata_import;
public:
	cli_metadata_reader(const wchar_t* _filename);
	~cli_metadata_reader(void);
	bool init();
	std::string format_token(mdToken token);
	void dump_type(mdTypeDef token);
};

