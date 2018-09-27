/** 
 *  �򵥵������ļ���ȡ�࣬ConfigFileReader.h
 */
#ifndef __CONFIG_FILE_READER_H__
#define __CONFIG_FILE_READER_H__

#include <map>
#include <string>

class CConfigFileReader
{
public:
    //CConfigFileReader(const char* filename);
    CConfigFileReader();
    ~CConfigFileReader();

    void InitFile(const char* filename);
    char* GetConfigName(const char* name);
    int SetConfigValue(const char* name, const char*  value);

private:
    void  _LoadFile(const char* filename);
    int   _WriteFIle(const char*filename = NULL);
    void  _ParseLine(char* line);
    char* _TrimSpace(char* name);

    bool                                m_load_ok;
    std::map<std::string, std::string>  m_config_map;
    std::string                         m_config_file;
};


#endif //!__CONFIG_FILE_READER_H__
