// This is the CPP file you will edit and turn in.
// Also remove these comments here and add your own.
// TODO: rewrite this comment

#include <iostream>
#include <string>
#include <random>
#include "console.h"
#include "gwindow.h"
#include "grid.h"
#include "simpio.h"
#include "strlib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "math.h" //for sqrt and exp in the optional Gaussian kernel
using namespace std;

static const int    WHITE = 0xFFFFFF;
static const int    BLACK = 0x000000;
static const int    GREEN = 0x00FF00;
static const double PI    = 3.14159265;

static void     doFauxtoshop(GWindow &gw, GBufferedImage &img);

static bool     openImageFromFilename(GBufferedImage& img, string filename);
static bool 	saveImageToFilename(const GBufferedImage &img, string filename);
static void     getMouseClickLocation(int &row, int &col);

//Custom fn declarations
static void 	fauxtoShopWork(GWindow &gw, GBufferedImage &img);
static void 	scatter(GWindow &gw, GBufferedImage &img);
static bool	 	canApplyEffect(GBufferedImage &img, int nRow, int nCol);
static int   	getRandomOffset(int currentIndex, int radius, int maxIndex);
static int 		getDifference(int pixel1, int pixel2);
static int 		isEdgePixel(GBufferedImage &img, Grid<int> original, int threshold, int r, int c);
static void 	edgeDetection(GWindow &gw, GBufferedImage &img);
/* STARTER CODE FUNCTION - DO NOT EDIT
 *
 * This main simply declares a GWindow and a GBufferedImage for use
 * throughout the program. By asking you not to edit this function,
 * we are enforcing that the GWindow have a lifespan that spans the
 * entire duration of execution (trying to have more than one GWindow,
 * and/or GWindow(s) that go in and out of scope, can cause program
 * crashes).
 */
int main() {
    GWindow gw;
    gw.setTitle("Fauxtoshop");
    gw.setVisible(true);
    GBufferedImage img;
    doFauxtoshop(gw, img);
    return 0;
}

/* This is yours to edit. Depending on how you approach your problem
 * decomposition, you will want to rewrite some of these lines, move
 * them inside loops, or move them inside helper functions, etc.
 *
 * TODO: rewrite this comment.
 */
static void doFauxtoshop(GWindow &gw, GBufferedImage &img) {
    cout << "Welcome to Fauxtoshop!" << endl;
    fauxtoShopWork(gw, img);
    int row, col;
    getMouseClickLocation(row, col);
    gw.clear();
}

static void fauxtoShopWork(GWindow &gw, GBufferedImage &img){
    int option = -1;
    while (true){
        string filename = getLine("Enter name of image file to open (or blank to quit): ");
        if (filename.empty()){
            return;
        }

        // Check if the filename is entered with an extension, add extension if not found
        int extensionLocation = filename.find(".jpg");
        if (extensionLocation == -1){
            filename +=".jpg";
        }
        cout << "Opening image file, may take a minute..." << endl;
        openImageFromFilename(img, filename);
        gw.setCanvasSize(img.getWidth(), img.getHeight());
        gw.add(&img,0,0);

        cout << "Which image filter would you like to apply?" << endl;
        string input = getLine("\t1 - Scatter\n"
                            "\t2 - Edge detection\n"
                            "\t3 - \"Green screen\" with another image\n"
                            "\t4 - Compare image with another image\n");
        try {
            option = stoi(input);
        }
        catch(...){
            cout << "Enter a valid number" << endl;
            continue;
        }

        switch (option) {
            case 1:
                scatter(gw, img);
                break;
            case 2:
                edgeDetection(gw, img);
                break;
            case 3:
            // Code for option 3
                break;
            case 4:
            // Code for option 4
                break;
            default:
                cout << "Please enter a valid option" << endl;
                break;
        }
    }
}

//---------------------- Scatter  Start-----------------------//

static void scatter(GWindow &gw, GBufferedImage &img){
    string input = getLine("Enter degree of scatter [1-100]: ");
    int radius = stoi(input);

    Grid<int> gridOriginal = img.toGrid();
    Grid<int> gridScattered(gridOriginal.nRows, gridOriginal.nCols);
    GBufferedImage scatteredImg(gridScattered.nRows, gridScattered.nCols);
    for(int i=0;i<gridScattered.nRows;i++){
        for(int j=0;j<gridScattered.nCols;j++){
            bool validPixel=false;
            int nrow, ncol;
            while (!validPixel){
                // Calculate new row and col with both positive and negative offsets
                nrow = i + getRandomOffset(i, radius, gridOriginal.nRows - 1);
                ncol = j + getRandomOffset(j, radius, gridOriginal.nCols - 1);

                validPixel= canApplyEffect(scatteredImg, nrow, ncol);
                if (validPixel){
                    break;
                }
            }
            gridScattered[i][j]=gridOriginal[nrow][ncol];
        }
    }
    gw.clear();
    scatteredImg.fromGrid(gridScattered);
    gw.add(&scatteredImg, 0, 0);

    input = getLine("Enter filename to save image (or blank to skip saving): ");
    if (input.length() == 0){
        gw.clear();
        cout << "" << endl;
        return;
    }
    saveImageToFilename(scatteredImg, input);
    gw.clear();
    cout << "" << endl;
    return;

}

static int getRandomOffset(int currentIndex, int radius, int maxIndex) {
    int minOffset = -std::min(currentIndex, radius); // How much we can move up/left
    int maxOffset = std::min(radius, maxIndex - currentIndex); // How much we can move down/right
    return minOffset + (rand() % (maxOffset - minOffset + 1));
}

static bool canApplyEffect(GBufferedImage &img, int nRow, int nCol){
    if (nRow < 0 || nRow > (img.getWidth()-1)){
        cout << "nRow out of bounds: " << nRow << endl;
        return false;
    }
    if (nCol < 0 || nCol > (img.getHeight()-1)){
        cout << "nCol out of bounds: " << nCol << endl;
        return false;
    }
    return true;
}


//---------------------- Scatter  End-----------------------//

//---------------------- Edge Detection Start-----------------------//

static void edgeDetection(GWindow &gw, GBufferedImage &img){
    string input = getLine("Enter threshold for edge detection: ");
    int threshold = stoi(input);

    Grid<int> gridOriginal = img.toGrid();
    Grid<int> gridEdged(gridOriginal.nRows, gridOriginal.nCols);
    GBufferedImage edgedImg(gridEdged.nRows, gridEdged.nCols);

    for(int i=0;i<gridEdged.nRows;i++){
        for(int j=0;j<gridEdged.nCols;j++){
            if(isEdgePixel(img, gridOriginal, threshold, i, j)){
                gridEdged[i][j]=BLACK;
            }else{
                gridEdged[i][j]=WHITE;
            }
        }
    }
    gw.clear();
    edgedImg.fromGrid(gridEdged);
    gw.add(&edgedImg, 0, 0);

    input = getLine("Enter filename to save image (or blank to skip saving): ");
    if (input.length() == 0){
        gw.clear();
        cout << "" << endl;
        return;
    }
    saveImageToFilename(edgedImg, input);
    gw.clear();
    cout << "" << endl;
    return;
}

static int isEdgePixel(GBufferedImage &img, Grid<int> original, int threshold, int r, int c){
    if(!img.inBounds(r, c)){
        return false;
    }
    int oPixel = original[r][c];
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, // Top-left, top, top-right
        { 0, -1},         { 0, 1}, // Left, right
        { 1, -1}, { 1, 0}, { 1, 1}  // Bottom-left, bottom, bottom-right
    };

    for (int i = 0; i < 8; i++){
        int r2 = r + directions[i][0];
        int c2 = c + directions[i][1];
        if(r2<0||r2>=original.nRows){
            continue;
        }
        if(c2<0||c2>=original.nCols){
            continue;
        }
        if(!img.inBounds(r2, c2)){
            continue;
        }
        int neighbour = original[r2][c2];
        int diff = getDifference(oPixel, neighbour);
        if (diff > threshold) {
            return true;
        }
    }

    return false;
}

static int getDifference(int pixel1, int pixel2){
    int p1r, p1g, p1b, p2r, p2g, p2b;
    GBufferedImage::getRedGreenBlue(pixel1, p1r, p1g, p1b);
    GBufferedImage::getRedGreenBlue(pixel2, p2r, p2g, p2b);
    int rDiff = abs(p1r-p2r);
    int gDiff = abs(p1g-p2g);
    int bDiff = abs(p1b-p2b);
    int maxDiff = max({rDiff, gDiff, bDiff});
    return maxDiff;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to open the image file 'filename'.
 *
 * This function returns true when the image file was successfully
 * opened and the 'img' object now contains that image, otherwise it
 * returns false.
 */
static bool openImageFromFilename(GBufferedImage& img, string filename) {
    try { img.load(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to save the image file to 'filename'.
 *
 * This function returns true when the image was successfully saved
 * to the file specified, otherwise it returns false.
 */
static bool saveImageToFilename(const GBufferedImage &img, string filename) {
    try { img.save(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Waits for a mouse click in the GWindow and reports click location.
 *
 * When this function returns, row and col are set to the row and
 * column where a mouse click was detected.
 */
static void getMouseClickLocation(int &row, int &col) {
    GMouseEvent me;
    do {
        me = getNextEvent(MOUSE_EVENT);
    } while (me.getEventType() != MOUSE_CLICKED);
    row = me.getY();
    col = me.getX();
}

/* OPTIONAL HELPER FUNCTION
 *
 * This is only here in in case you decide to impelment a Gaussian
 * blur as an OPTIONAL extension (see the suggested extensions part
 * of the spec handout).
 *
 * Takes a radius and computes a 1-dimensional Gaussian blur kernel
 * with that radius. The 1-dimensional kernel can be applied to a
 * 2-dimensional image in two separate passes: first pass goes over
 * each row and does the horizontal convolutions, second pass goes
 * over each column and does the vertical convolutions. This is more
 * efficient than creating a 2-dimensional kernel and applying it in
 * one convolution pass.
 *
 * This code is based on the C# code posted by Stack Overflow user
 * "Cecil has a name" at this link:
 * http://stackoverflow.com/questions/1696113/how-do-i-gaussian-blur-an-image-without-using-any-in-built-gaussian-functions
 *
 */
static Vector<double> gaussKernelForRadius(int radius) {
    if (radius < 1) {
        Vector<double> empty;
        return empty;
    }
    Vector<double> kernel(radius * 2 + 1);
    double magic1 = 1.0 / (2.0 * radius * radius);
    double magic2 = 1.0 / (sqrt(2.0 * PI) * radius);
    int r = -radius;
    double div = 0.0;
    for (int i = 0; i < kernel.size(); i++) {
        double x = r * r;
        kernel[i] = magic2 * exp(-x * magic1);
        r++;
        div += kernel[i];
    }
    for (int i = 0; i < kernel.size(); i++) {
        kernel[i] /= div;
    }
    return kernel;
}
