#include "../include/SPU_fileHandling.h"


SPU_fileError putBinFileToBuffer(uint8_t** buffer, const size_t size, const char* FILE_NAME)
{
    uint8_t* tempBuf = (uint8_t*)calloc(size, 1);
    if (tempBuf == NULL)
    {
        return SPU_FILE_MEM_ALLOCATION_ERROR;
    }

    FILE* file = fopen(FILE_NAME, "rb");
    if (file == NULL)
    {
        free(tempBuf);
        return SPU_FILE_FILE_OPEN_ERROR;
    }

    size_t sizeRef = fread(tempBuf, 1, size, file);
    if (sizeRef != size)
    {
        fclose(file);
        free(tempBuf);
        return SPU_FILE_FREAD_ERROR;
    }

    fclose(file);

    *buffer = tempBuf;
 
    return SPU_FILE_NO_ERROR;
}


SPU_fileError getFileSize(const char* fileName, size_t* size)
{
    struct stat bf = {};
    int error = stat(fileName, &bf);
    if (error == -1)
    {
        return SPU_FILE_STAT_ERROR;
    }

    *size = bf.st_size;
    return SPU_FILE_NO_ERROR;
}


SPU_fileError createBuffer(uint8_t** buffer, const char* inputFileName)
{
    size_t bufferSize = 0;
    SPU_fileError fileErr = getFileSize(inputFileName, &bufferSize);
    if (fileErr != SPU_FILE_NO_ERROR)
        return fileErr;

    fileErr = putBinFileToBuffer(buffer, bufferSize, inputFileName);
    if (fileErr != SPU_FILE_NO_ERROR)
        return fileErr;

    return SPU_FILE_NO_ERROR;
}
