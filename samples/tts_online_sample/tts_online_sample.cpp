/*
* �����ϳɣ�Text To Speech��TTS�������ܹ��Զ�����������ʵʱת��Ϊ������
* ��Ȼ��������һ���ܹ����κ�ʱ�䡢�κεص㣬���κ����ṩ������Ϣ�����
* ��Ч����ֶΣ��ǳ�������Ϣʱ���������ݡ���̬���º͸��Ի���ѯ������
*/

#include "charsetConv.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <vector>

#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#include "readFile.h"
#pragma execution_character_set("utf-8")
#pragma comment(lib,"WinMM.Lib")

#ifdef _WIN64
#pragma comment(lib,"../../libs/msc_x64.lib")//x64
#else
#pragma comment(lib,"../../libs/msc.lib")//x86
#endif

BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);


	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
	return TRUE;
}


/* wav��Ƶͷ����ʽ */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = ��һ���ṹ��Ĵ�С : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = ͨ���� : 1
	int				samples_per_sec;        // = ������ : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = ÿ���ֽ��� : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = ÿ�������ֽ��� : wBitsPerSample / 8
	short int       bits_per_sample;        // = ����������: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = �����ݳ��� : FileSize - 44 
} wave_pcm_hdr;

/* Ĭ��wav��Ƶͷ������ */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};
/* �ı��ϳ� */
int text_to_speech(const char* src_text, const char* des_path, const char* params)
{

	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_path)
	{
		printf("params is error%s!\n", src_text);
		return ret;
	}
	fopen_s(&fp,des_path, "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path);
		return ret;
	}
	/* ��ʼ�ϳ� */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	printf("���ںϳ� ...\n");
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //���wav��Ƶͷ��ʹ�ò�����Ϊ16000
	while (1) 
	{
		/* ��ȡ�ϳ���Ƶ */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //����data_size��С
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
		printf(">");
		Sleep(150); //��ֹƵ��ռ��CPU
	}
	printf("\n");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* ����wav�ļ�ͷ���ݵĴ�С */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* ��������������д���ļ�ͷ��,��Ƶ�ļ�Ϊwav��ʽ */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //д��size_8��ֵ
	fseek(fp, 40, 0); //���ļ�ָ��ƫ�Ƶ��洢data_sizeֵ��λ��
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //д��data_size��ֵ
	fclose(fp);
	fp = NULL;
	/* �ϳ���� */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	return ret;
}





int main(int argc, char* argv[])
{
	int         ret                  = MSP_SUCCESS;
	const char* login_params         = "appid = 5c0e2753, work_dir = .";//��¼����,appid��msc���,��������Ķ�
	/*
	* rdn:           �ϳ���Ƶ���ַ�����ʽ
	* volume:        �ϳ���Ƶ������
	* pitch:         �ϳ���Ƶ������
	* speed:         �ϳ���Ƶ��Ӧ������
	* voice_name:    �ϳɷ�����
	* sample_rate:   �ϳ���Ƶ������
	* text_encoding: �ϳ��ı������ʽ
	*
	*/
	const char* session_begin_params = "voice_name = xiaofeng, text_encoding = gb2312, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
	const char* filename             = "tts_sample.wav"; //�ϳɵ������ļ�����
	const char* inFile;
	const char* outFile;
	/* �û���¼ */
	ret = MSPLogin(NULL, NULL, login_params); //��һ���������û������ڶ������������룬�����������ǵ�¼�������û������������http://www.xfyun.cnע���ȡ
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		MSPLogout(); //�˳���¼
		exit(1);//��¼ʧ�ܣ��˳���¼
	}
	
	if(argc==3){
		inFile =argv[1];
		outFile = argv[2];
	}
	else {
		printf("tts.exe inFile outFile\n");
		exit(2);
	}
	std::vector<char> *content=new std::vector<char>();
	readUtf8File(inFile, content);

	/* �ı��ϳ� */

	printf("start ...\n");
	const char* text = content->data();
	printf("%s",text);
	ret = text_to_speech(text, outFile, session_begin_params);

	delete content;
	if (MSP_SUCCESS != ret)
	{
		printf("text_to_speech failed, error code: %d.\n", ret);
	}

	printf("complete\n");

	//char*  s = "tts_sample.wav";
	//wchar_t wText[30] = { 0 };
	//MByteToWChar(s, wText, sizeof(wText) / sizeof(wText[0]));
	//PlaySound(wText, NULL, SND_FILENAME | SND_SYNC);


	
	return 0;
}
