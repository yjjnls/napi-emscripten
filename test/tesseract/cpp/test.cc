#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <assert.h>
int main(int argc, char *argv[])
{
    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = NULL;
    if (argc <= 1)
        image = pixRead("test.bmp");
    else
        image = pixRead(argv[1]);
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    char target[] = "0:00:05.233";
    printf("*************************************\n");
    printf("target:\t\t%s\nOCR output:\t%s", target, outText);
    printf("*************************************\n");
    int res = strncmp(outText, target, strlen(target));
    if (res != 0) {
        printf("OCR result is wrong!\n\n");
        throw "OCR result is wrong!";
    }

    // Destroy used object and release memory
    api->End();
    delete[] outText;
    pixDestroy(&image);

    return 0;
}