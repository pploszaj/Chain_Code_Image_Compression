#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

class ChainCode {

public:
    struct point {
        int row;
        int col;
    };

    struct CCproperty {
        int label;
        int numPixels;
        int minRow;
        int minCol;
        int maxRow;
        int maxCol;
    };

    int numCC;
    CCproperty CC;
    int numRows;
    int numCols;
    int minVal;
    int maxVal;
    int** imgAry;
    int** boundaryAry;
    int** CCAry;
    point coordOffset[8];
    int zeroTable[8];
    point startP;
    point currentP;
    point nextP;
    int lastQ;
    int nextDir;
    int PchainDir;

    ChainCode(int numRows, int numCols, int minVal, int maxVal, int numCC) : numRows(numRows), numCols(numCols), minVal(minVal), maxVal(maxVal), numCC(numCC){}


    void loadImage(ifstream& labelFile){
        if(!labelFile){
            std::cerr << "File is not open" << std::endl;
            return;
        }

        int pixelVal;

        for (int i = 1; i < numRows; i++) {
            for (int j = 1; j < numCols; j++) {
                if (labelFile >> pixelVal) {
                    imgAry[i][j] = pixelVal;
                }
            }
        }
    }

    void reformatPrettyPrint(int** inAry, ofstream& outFile1){
        outFile1 << numRows << numCols << minVal << maxVal << endl;
        string str = to_string(maxVal);
        int width = str.length();
        for(int i = 0; i < numRows; i++){
            for(int j = 0; j < numCols; j++){
                if(inAry[i][j] > 0){
                    outFile1 << inAry[i][j];
                } else {
                    outFile1 << '.';
                }
                string str2 = to_string(inAry[i][j]);
                int WW = str2.length();
                while (WW <= width){
                    outFile1 << " ";
                    WW++;
                }
            }
            outFile1 << endl;
        }
    }


};

int main (int argc, char* argv[]){

    if (argc < 3) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        return 1;
    }

    ifstream labelFile(argv[1]);
    ifstream propFile(argv[2]);
    ofstream outFile1(argv[3]);
    ofstream debugFile(argv[4]);
    ofstream chainCodeFile(argv[5]);
    ofstream boundaryFile(argv[6]);

    int numRows, numCols, minVal, maxVal, numCC;
    labelFile >> numRows >> numCols >> minVal >> maxVal;
    propFile >> numRows >> numCols >> minVal >> maxVal;
    propFile >> numCC;
    ChainCode chainCode(numRows, numCols, minVal, maxVal, numCC);
    chainCode.zeroFramed();
    chainCode.loadImage(labelFile);
    chainCode.reformatPrettyPrint(chainCode.imgAry, outFile1);

    chainCodeFile << numRows << numCols << minVal << maxVal << endl;
    chainCodeFile << numCC;

    propFile >> chainCode.CC.label >> chainCode.CC.numPixels >> chainCode.CC.minRow >> chainCode.CC.minCol >> chainCode.CC.maxRow >> chainCode.CC.maxCol;




}