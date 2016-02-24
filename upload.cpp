#include "upload.h"
#include "logger.h"
#include <fcgi_stdio.h>

#define DEAL_BUF_LEN 1024
#define SIGN_CODE_LEN 100
#define FILE_NAME_LEN 64

enum
{
    STATE_START,
    STATE_GET_SIGN_CODE,
    STATE_GET_FILE_NAME,
    STATE_GET_FILE_START,
    STATE_GET_FILE_CONTENT,
    STATE_CHECK_END,
    STATE_END
};

int upload_file_save_as(const char *saveas)
{
    FILE *fp; 
    int getState = STATE_START;
    int contentLength; /* uploaded file content length */
    int nowReadLen;
    int signCodeLen;
    int tmpLen;
    char *nowReadP;
    char *nowWriteP;
    char dealBuf[DEAL_BUF_LEN];
    char signCode[SIGN_CODE_LEN]; /* http boundary */
    char tmpSignCode[SIGN_CODE_LEN];
    char fileName[FILE_NAME_LEN];
    memset(dealBuf, 0, DEAL_BUF_LEN);
    memset(signCode, 0, SIGN_CODE_LEN);
    memset(fileName, 0, FILE_NAME_LEN);
    nowReadLen = 0;
    if((char *)getenv("CONTENT_LENGTH")!=NULL)
    {
        contentLength = atoi((char *)getenv("CONTENT_LENGTH"));
    }
    else
    {
        return EUPLOAD_NO_DATA;
    }

    while(contentLength > 0)
    {
        if(contentLength >= DEAL_BUF_LEN)
        {
            nowReadLen = DEAL_BUF_LEN;
        }
        else
        {
            nowReadLen = contentLength;
        }
        contentLength -= nowReadLen;
        if(fread(dealBuf, sizeof(char), nowReadLen, FCGI_stdin) != (size_t)nowReadLen)
        {
            log_error("read error %d", nowReadLen);
            return EUPLOAD_READ;
        }
        nowReadP = dealBuf;
        while(nowReadLen > 0)
        {
            switch (getState)
            {
                case STATE_START:
                    nowWriteP = signCode;
                    getState = STATE_GET_SIGN_CODE;
                case STATE_GET_SIGN_CODE:
                    if(strncmp(nowReadP, "\r\n", 2) == 0)
                    {
                        signCodeLen = nowWriteP - signCode;
                        nowReadP++;
                        nowReadLen--;
                        *nowWriteP = 0;
                        getState = STATE_GET_FILE_NAME;
                    }
                    else
                    {
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                    }
                    break;
                case STATE_GET_FILE_NAME:
                    if(strncmp(nowReadP,"filename=",strlen("filename=")) == 0)
                    {
                        nowReadP += strlen("filename=");
                        nowReadLen -= strlen("filename=");
                        nowWriteP = fileName + strlen(saveas);
                        while(*nowReadP != '\r')
                        {
                            if(*nowReadP == '\\' || *nowReadP == '/')
                            {
                                nowWriteP = fileName + strlen(saveas);
                            }
                            else if(*nowReadP != '\"')
                            {
                                *nowWriteP = *nowReadP;
                                nowWriteP++;
                            }
                            nowReadP++;
                            nowReadLen--;
                        }
                        *nowWriteP = 0;
                        nowReadP++;
                        nowReadLen--;
                        getState = STATE_GET_FILE_START;
                        memcpy(fileName, saveas, strlen(saveas));
                        if((fp = fopen(fileName, "w")) == NULL)
                        {
                            log_error("open file %s error %d", fileName, errno);
                            return EUPLOAD_WRITE;
                        }
                    }
                    break;
                case STATE_GET_FILE_START:
                    if(strncmp(nowReadP, "\r\n\r\n", 4) == 0)
                    {
                        nowReadP += 3;
                        nowReadLen -= 3;
                        getState = STATE_GET_FILE_CONTENT;
                    }
                    break;
                case STATE_GET_FILE_CONTENT:
                    if(*nowReadP != '\r')
                    {
                        fputc(*nowReadP,fp);
                    }
                    else
                    {
                        if(nowReadLen >= (signCodeLen + 2)) //\r\n=2
                        {
                            if(strncmp(nowReadP + 2, signCode, signCodeLen) == 0)
                            {
                                getState = STATE_END;
                                nowReadLen = 1;
                            }
                            else
                            {
                                fputc(*nowReadP,fp);
                            }
                        }
                        else
                        {
                            getState = STATE_CHECK_END;
                            nowWriteP = tmpSignCode;
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen = 1;
                        }
                    }
                    break;
                case STATE_CHECK_END:
                    if(*nowReadP != '\r')
                    {
                        if(tmpLen < signCodeLen + 2)
                        {
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen++;
                            if(tmpLen == signCodeLen + 2)
                            {
                                *nowWriteP = 0;
                                if((tmpSignCode[1] == '\n')&&(strncmp(tmpSignCode + 2,signCode,signCodeLen) == 0))
                                {
                                    getState = STATE_END;
                                    nowReadLen = 1;
                                }
                                else
                                {
                                    //fprintf(fp,tmpSignCode);
                                    fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                                    getState = STATE_GET_FILE_CONTENT;
                                }
                            }
                        }
                    }
                    else
                    {
                        *nowWriteP = 0;
                        //fprintf(fp,tmpSignCode);
                        fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                        nowWriteP = tmpSignCode;
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                        tmpLen = 1;
                    }
                    break;
                case STATE_END:
                    nowReadLen = 1;
                    break;
                default:break;
            }
            nowReadLen--;
            nowReadP++;
        }
    }
    if(fp != NULL)
    {
        fclose(fp);
    }
    return EUPLOAD_SUCCESS;
}
