using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace TPanda.PDFBuilderCS
{
    public class PDFBuilder : IDisposable {
        [DllImport("PDFBuilder.dll")]
        private static extern IntPtr CreatePDfBuilder();
        [DllImport("PDFBuilder.dll")]
        private static extern void GetPdfData(IntPtr pdfBuilder, IntPtr dst);
        [DllImport("PDFBuilder.dll")]
        private static extern int GetPdfDataSize(IntPtr pdfBuilder);
        [DllImport("PDFBuilder.dll")]
        private static extern void DeletePDfBuilder(IntPtr pdfBuilder);
        [DllImport("PDFBuilder.dll")]
        private static extern bool AddJpgToPdf(IntPtr pdfBuilder, IntPtr jpeg_bytes, int jpeg_bytes_len);
        [DllImport("PDFBuilder.dll")]
        private static extern void BuildPdfData(IntPtr pdfBuilder);

        private IntPtr handle;
        private byte[] builtPdfData;
        private bool disposed = false;

        public PDFBuilder() {
            handle = PDFBuilder.CreatePDfBuilder();
            builtPdfData = null;
        }

        public bool tryAddFromJpegFile(string file_path) {
            using(Image image = Image.FromFile(file_path))
                if(!image.RawFormat.Equals(ImageFormat.Jpeg)) return false;

            bool b;
            using( FileStream fs = new FileStream(file_path, FileMode.Open, FileAccess.Read) ) {
                byte[] bytes = new byte[fs.Length];
                fs.Read(bytes, 0, bytes.Length);
                IntPtr ptr = PDFBuilder.CreateUnmanagedByteArrayPtr(bytes);
                b = AddJpgToPdf(handle, ptr, bytes.Length);
                Marshal.FreeCoTaskMem(ptr);
            }
            return b;
        }

        public bool tryAddImage(Image image) {
            bool b;
            using(MemoryStream memStrm = new MemoryStream()) {
                image.Save(memStrm, ImageFormat.Jpeg);
                byte[] buff = new byte[memStrm.Length];
                memStrm.Position = 0;
                memStrm.Read(buff, 0, (int)memStrm.Length);
                IntPtr ptr = PDFBuilder.CreateUnmanagedByteArrayPtr(buff);
                b = AddJpgToPdf(handle, ptr, buff.Length);
                Marshal.FreeCoTaskMem(ptr);
            }
            return b;
        }

        public byte[] build() {
            if(this.builtPdfData != null) return this.builtPdfData;
            PDFBuilder.BuildPdfData(handle);
            byte[] data = new byte[PDFBuilder.GetPdfDataSize(handle)];
            IntPtr dataPtr = PDFBuilder.CreateUnmanagedByteArrayPtr(data);
            PDFBuilder.GetPdfData(handle, dataPtr);
            Marshal.Copy(dataPtr, data, 0, data.Length);
            Marshal.FreeCoTaskMem(dataPtr);
            this.builtPdfData = data;
            return data;
        }

        public void save(string output_file_path) {
            using( BinaryWriter bWriter = new BinaryWriter(new FileStream(output_file_path, FileMode.Create)) ) {
                if(this.builtPdfData != null)
                    bWriter.Write(this.builtPdfData);
                else
                    bWriter.Write(build());
            }
        }

        public void Dispose() {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing) {
            if(!this.disposed) {
                PDFBuilder.DeletePDfBuilder(handle);
                handle = IntPtr.Zero;
                disposed = true;
            }
        }
        private static IntPtr CreateUnmanagedByteArrayPtr(byte[] bytes) {
            int size = Marshal.SizeOf(typeof(byte)) * bytes.Length;
            IntPtr ptr = Marshal.AllocCoTaskMem(size);
            Marshal.Copy(bytes, 0, ptr, bytes.Length);
            return ptr;
        }
        ~PDFBuilder() {
            Dispose(false);
        }
    }
}
