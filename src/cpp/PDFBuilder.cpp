#define BUILD_PDFBUILDER_DLL

#include "header/PDFBuilder.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>

const int PDF_DPI = 72;
const int MONITOR_DPI = 96;

float nFloor(float a, int n) {
    int keta = std::pow(10, n);
    int b = a * keta;
    return ((float)b) / keta;
}

struct JpgMeta {
    int width;
    int height;
    int num_of_col;
};

bool getJpgMeta(char jpg_bytes[], int data_len, JpgMeta* meta) {
    int blk_len = jpg_bytes[4] * 256 + jpg_bytes[5];
    int index = 4;
    while (index + blk_len < data_len) {
        index += blk_len;
        if((unsigned char)(jpg_bytes[index]) != 0xff) return false;
        if((unsigned char)(jpg_bytes[index+1]) == 0xC0 || (unsigned char)(jpg_bytes[index+1]) == 0xC2) {
            meta->width  = (unsigned char)(jpg_bytes[index+7]) * 256 + (unsigned char)(jpg_bytes[index+8]);
            meta->height = (unsigned char)(jpg_bytes[index+5]) * 256 + (unsigned char)(jpg_bytes[index+6]);
            meta->num_of_col = (unsigned char)(jpg_bytes[index+9]);
            return true;
        }
        else {
            index += 2;
            blk_len = (unsigned char)(jpg_bytes[index]) * 256 + (unsigned char)(jpg_bytes[index+1]);
        }
    }
    return false;
}

std::string zero_padding(int val, int keta) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(keta) << val;
    return oss.str();
}

class _PDFBuilder : public PDFBuilder {
private:
    std::vector<unsigned char> data;
    std::map<int, int> xref;
    std::vector<int> refs;
    std::vector<int> pages;
    int catalog;
    int root;
    int num_of_img;
    int data_tell;
    bool isBuilt;

    void _writeJpg(char jpg[], int len) {
        for(size_t i = 0; i < len; i++) {
            data.push_back((unsigned char)jpg[i]);
        }
        data_tell += len;
    }
    void _write(std::string str) {
        data.insert(data.begin()+data_tell, str.begin(), str.end());
        data_tell += str.length();
    }
    void _write(int i) {
        std::ostringstream oss;
        oss << i;
        _write(oss.str());
    }
    void _write(std::map<std::string, std::string> dic) {
        _write(_to_str_dic(dic));
    }
    std::string _to_str_dic(std::map<std::string, std::string> dic) {
        std::ostringstream oss;
        oss << "<<\n";
        for(auto const& [k, v] : dic) {
            oss << "/" << k << " " << v << " ";
        }
        oss << ">>\n";
        return oss.str();
    }
    std::string _to_str_pages_value(int i, int j) {
        std::ostringstream oss;
        oss << (i+1) << " " << j << " R";
        return oss.str();
    }
    std::string _to_str_page_size_value(float f1, float f2) {
        std::ostringstream oss;
        oss << nFloor(f1, 2) << " 0 0 " << nFloor(f2, 2) << " 0 0 cm\n";
        return oss.str();
    }
    std::string _to_str_mediabox_value(int f1, float f2) {
        std::ostringstream oss;
        oss << "[0.0 0.0 " << nFloor(f1, 2) << " " << nFloor(f2, 2) << "]";
        return oss.str();
    }
    std::string _get_str_kids_value() {
        std::ostringstream oss;
        oss << "[";
        for(size_t i = 0; i < pages.size(); i++) {
            oss << _to_str_pages_value(pages.at(i), 0) << " ";
        }
        oss << "]";
        return oss.str();
    }
    int _reserve_obj(){
        int i = refs.size() + 1;
        refs.push_back(i);
        return i;
    }
    int _bgn_obj(int reserved) {
        int i;
        if(reserved != -1) i = reserved;
        else {
            i = this->refs.size() + 1;
            refs.push_back(i);
        }
        xref.insert({i, data_tell});
        std::ostringstream oss;
        oss << (i+1) << " 0 obj\n";
        _write(oss.str());
        return i;
    }
    void _end_obj() {
        _write("endobj\n");
    }
public:
    _PDFBuilder() {
        num_of_img = 0;
        data_tell = 0;
        isBuilt = false;

        _write("%PDF-1.7\n");
        catalog = _bgn_obj(false);
        root = _reserve_obj();
        _write({
            {"Type", "/Catalog"},
            {"Pages", _to_str_pages_value(root, 0)}
        });
        _end_obj();
    }
    bool addImage(std::vector<char> jpg_bytes) {
        return addJpgFromBytes(jpg_bytes.data(), jpg_bytes.size());
    }
    bool addJpgFromBytes(char jpg_bytes[], int data_len) {
        JpgMeta meta;
        if(!getJpgMeta(jpg_bytes, data_len, &meta)) return false;
        float page_w = (float)meta.width  * PDF_DPI / MONITOR_DPI;
        float page_h = (float)meta.height * PDF_DPI / MONITOR_DPI;

        int img_index = _bgn_obj(-1);
        std::string col_space;
        if(meta.num_of_col == 1) col_space = "/DeviceGray";
        else col_space = "/DeviceRGB";
        _write({
            {"Type","/XObject"},
            {"Subtype","/Image"},
            {"Filter","/DCTDecode"},
            {"BitsPerComponent","8"},
            {"ColorSpace",col_space},
            {"Width",std::to_string(meta.width)},
            {"Height",std::to_string(meta.height)},
            {"Length",std::to_string(data_len)}
        });
        _write("stream\n");
        _writeJpg(jpg_bytes, data_len);
        _write("endstream\n");
        _end_obj();

        int cts_index = _bgn_obj(-1);
        _write({
            {"Length", _to_str_pages_value(cts_index + 1, 0)}
        });
        _write("stream\n");
        int bgn_fp = data_tell;
        _write("q\n1 0 0 1 0.00 0.00 cm\n");
        _write(_to_str_page_size_value(page_w, page_h));
        _write("/I"+std::to_string(num_of_img)+" Do\nQ\n");
        int len = data_tell - bgn_fp;
        _write("endstream\n");
        _end_obj();

        _bgn_obj(-1);
        _write(std::to_string(len) + "\n");
        _end_obj();

        int rsc_index = _bgn_obj(-1);
        std::string procset;
        if(meta.num_of_col == 1) procset = "ImageB";
        else procset = "ImageC";
        _write({
            {"ProcSet", "[/PDF /"+procset+"]"},
            {"XObject", _to_str_dic({
                {"I"+std::to_string(num_of_img), _to_str_pages_value(img_index, 0) }
            })}
        });
        _end_obj();

        int page_index = _bgn_obj(-1);
        pages.push_back(page_index);
        _write({
            {"Type","/Page"},
            {"Parent",_to_str_pages_value(root, 0)},
            {"MediaBox",_to_str_mediabox_value(page_w, page_h)},
            {"Contents",_to_str_pages_value(cts_index, 0)},
            {"Resources",_to_str_pages_value(rsc_index, 0)}
        });
        _end_obj();

        num_of_img++;
        return true;
    }
    bool addJpgFromFile(const char input_jpgFile_path[]) {
        std::ifstream f(input_jpgFile_path, std::ios::in | std::ios::binary);
        if(!f) return false; 
        f.seekg(0, std::ios::end);
        long long int size = f.tellg();
        f.seekg(0);
        char jpg_bytes[size];
        f.read(jpg_bytes, size);
        f.close();
        return addJpgFromBytes(jpg_bytes, size);;
    }
    void build() {
        if(isBuilt) return;
        _bgn_obj(root);
        _write({
            {"Type","/Pages"},
            {"Kids",_get_str_kids_value()},
            {"Count",std::to_string(pages.size())}
        });
        _end_obj();

        int xref_fp = data_tell;
        _write("xref\n");
        _write("0 " + std::to_string(xref.size()+1) + "\n");
        _write("0000000000 65535 f\n");
        for(auto const& [k,v] : xref) {
            _write(zero_padding(v, 10) + " 00000 n\n");
        }
        _write("trailer\n");
        _write({
            {"Root", _to_str_pages_value(catalog, 0)},
            {"Size", std::to_string(xref.size() + 1)}
        });
        _write("startxref\n"+std::to_string(xref_fp)+"\n");
        _write("%%EOF\n");
        isBuilt = true;
    }
    void save(const char output_file_path[]) {
        if(!isBuilt) build();
        int len = getPdfDataSize();
        char data[len];
        getPdfData(data);

        std::ofstream out(output_file_path, std::ios::binary | std::ios::out);
        std::ostreambuf_iterator<char> it_ofs(out);
        std::vector<unsigned char> a;
        for(int i = 0; i < len; i++) {
            a.push_back((unsigned char)data[i]);
        }
        std::copy(std::cbegin(a), std::cend(a), it_ofs);
        out.close();
    }
    int getPdfDataSize() {
        return data.size();
    }
    void getPdfData(char dst[]) {
        memcpy(dst, data.data(), data.size());
    }
    ~_PDFBuilder() {}
};

extern "C" {
    DLL_FUNCTION PDFBuilder* __stdcall CreatePDfBuilder() {
        return new _PDFBuilder();
    }
    DLL_FUNCTION void __stdcall DeletePDfBuilder(PDFBuilder* pdfBuilder) {
        if(pdfBuilder) {
            delete pdfBuilder;
            pdfBuilder = nullptr;
        }
    }
    DLL_FUNCTION void __stdcall GetPdfData(PDFBuilder* pdfBuilder, char dst[]) {
        pdfBuilder->getPdfData(dst); 
    }
    DLL_FUNCTION bool __stdcall AddJpgToPdf(PDFBuilder* pdfBuilder, char jpeg_bytes[], int jpeg_bytes_len) {
        return pdfBuilder->addJpgFromBytes(jpeg_bytes, jpeg_bytes_len);
    }
    DLL_FUNCTION void BuildPdfData(PDFBuilder* pdfBuilder) {
        pdfBuilder->build();
    }
    DLL_FUNCTION int __stdcall GetPdfDataSize(PDFBuilder* pdfBuilder) {
        return pdfBuilder->getPdfDataSize();
    }
}
