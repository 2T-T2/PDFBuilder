# PDFBuilder
C#版は、C++版のdllを同じディレクトリに置く必要がある。<br>
<br>
<a href="https://github.com/2T-T2/PDFBuilder/releases/download/v.0.0.1/Cpp_PDFBuilder.zip">C++版コンパイル済みライブラリ</a>(GCCを使用)<br>
<a href="https://github.com/2T-T2/PDFBuilder/releases/download/v.0.0.1/Cs_PDFBuilder.zip">C#版コンパイル済みライブラリ</a>v4.0.30319<br>
<br>
## C++版サンプル
<pre>
<code>

#include "header/PDFBuilder.h"

int main(int argc, char const *argv[]) {
    const char output_file_name[] = "test1.pdf";
    PDFBuilder* builder = CreatePDfBuilder();
    for(int i = 1; i < argc; i++) {
        builder->addJpgFromFile(argv[i]);
    }
    builder->build();
    builder->save(output_file_name);
    DeletePDfBuilder(builder);
    return 0;
}

</code>
</pre>
<br>

## C版サンプル
<pre>
<code>

#include "header/PDFBuilder.h"
#include &lt;stdio.h>

int main(int argc, char const *argv[]) {
    const char output_file_name[] = "test1.pdf";
    PDFBuilder* builder = CreatePDfBuilder();
    for(int i = 1; i < argc; i++) {
        FILE* fp = fopen(argv[i], "rb");
        if(fp == NULL) return -1;
        long long int size;
        if(fseek(fp, 0, SEEK_END) == 0) {
            if(fgetpos(fp, &size) != 0) return -1;
        }
        fseek(fp, 0, 0);
        char jpg_bytes[size];
        fread(jpg_bytes, sizeof(char), sizeof(char)*size, fp);
        fclose(fp);
        AddJpgToPdf(builder, jpg_bytes, size);
    }

    BuildPdfData(builder);
    int len = GetPdfDataSize(builder);
    char data[len];
    GetPdfData(builder, data);
    DeletePDfBuilder(builder);

    FILE* fp = fopen(output_file_name, "wb");
    fwrite(data, sizeof(char), sizeof(char)*len, fp);
    fclose(fp);

    return 0;
}

</code>
</pre>
<br>

## CSharp版サンプル
<pre>
<code>

using System;
using TPanda.PDFBuilderCS;

class Sample {
    public static void Main(string[] args) {
        string output_file_name = "test1.pdf";
        PDFBuilder pdfBuilder = new PDFBuilder();
        for(int i = 0; i < args.Length; i++) {
            // using(Image image = Image.FromFile(args[i]))
            //     pdfBuilder.tryAddImage(image);
            pdfBuilder.tryAddFromJpegFile(args[i]);
        }
        pdfBuilder.save(output_file_name);

        pdfBuilder.Dispose();
    }
}

</code>
</pre>
