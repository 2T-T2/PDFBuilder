#pragma once

#ifdef BUILD_PDFBUILDER_DLL
#define DLL_FUNCTION __declspec(dllexport)
#else
#define DLL_FUNCTION __declspec(dllimport)
#endif

#ifdef __cplusplus
class PDFBuilder {
    public:
    virtual bool addJpgFromBytes(char jpg_bytes[], int data_len) = 0;
    virtual bool addJpgFromFile(const char input_jpgFile_path[]) = 0;
    virtual void build() = 0;
    virtual int  getPdfDataSize() = 0;
    virtual void getPdfData(char dst[]) = 0;
    virtual void save(const char output_file_path[]) = 0;
};
#else
typedef struct {} PDFBuilder;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    DLL_FUNCTION PDFBuilder* __stdcall CreatePDfBuilder();
    DLL_FUNCTION void __stdcall GetPdfData(PDFBuilder* pdfBuilder, char dst[]);
    DLL_FUNCTION int __stdcall GetPdfDataSize(PDFBuilder* pdfBuilder);
    DLL_FUNCTION void __stdcall DeletePDfBuilder(PDFBuilder* pdfBuilder);
    DLL_FUNCTION bool __stdcall AddJpgToPdf(PDFBuilder* pdfBuilder, char jpeg_bytes[], int jpeg_bytes_len);
    DLL_FUNCTION void BuildPdfData(PDFBuilder* pdfBuilder);
#ifdef __cplusplus
}
#endif
