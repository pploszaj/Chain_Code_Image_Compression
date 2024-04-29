#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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
    point coordOffset[8] = {{0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}};
    int zeroTable[8] = {6, 0, 0, 2, 2, 4, 4, 6};
    point startP;
    point currentP;
    point nextP;
    int lastQ;
    int nextDir;
    int PchainDir;

    ChainCode(int numRows, int numCols, int minVal, int maxVal, int numCC) : numRows(numRows), numCols(numCols), minVal(minVal), maxVal(maxVal), numCC(numCC){
        initializeArrays();
    }

    ~ChainCode() {
        deleteArrays();
    }


    void loadImage(ifstream& labelFile){
        if(!labelFile){
            std::cerr << "File is not open" << std::endl;
            return;
        }

        int pixelVal;

        for (int i = 1; i < numRows + 1; i++) {
            for (int j = 1; j < numCols + 1; j++) {
                if (labelFile >> pixelVal) {
                    imgAry[i][j] = pixelVal;
                }
            }
        }
    }

    void reformatPrettyPrint(int** inAry, ofstream& outFile1){
        outFile1 << numRows << " " << numCols << " " << minVal << " " << maxVal << endl;
        string str = to_string(maxVal);
        int width = str.length();
        for(int i = 2; i < numRows; i++){
            for(int j = 2; j < numCols; j++){
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

    void initializeArrays() {
        imgAry = new int*[numRows + 2];
        boundaryAry = new int*[numRows + 2];
        CCAry = new int*[numRows + 2];

        for (int i = 0; i < numRows + 2; ++i) {
            imgAry[i] = new int[numCols + 2];
            boundaryAry[i] = new int[numCols + 2];
            CCAry[i] = new int[numCols + 2];
        }
    }

    void deleteArrays() {
        for (int i = 0; i < numRows + 2; ++i) {
            delete[] imgAry[i];
            delete[] boundaryAry[i];
            delete[] CCAry[i];
        }
        delete[] imgAry;
        delete[] boundaryAry;
        delete[] CCAry;
    }

    // Method stubs
    void zeroFramed() {
        for (int col = 0; col < numCols + 2; col++) {
            imgAry[0][col] = 0;
            imgAry[numRows + 1][col] = 0;
        }

        for (int row = 0; row < numRows + 2; row++) {
            imgAry[row][0] = 0;
            imgAry[row][numCols + 1] = 0;
        }
    }

    void clearCCAry() {
        for (int i = 0; i < numRows + 2; i++) {
            for (int j = 0; j < numCols + 2; j++) {
                CCAry[i][j] = 0;
            }
        }
    }

    void loadCCAry() {
        for (int i = 1; i <= numRows; i++) {
            for (int j = 1; j <= numCols; j++) {
                if (imgAry[i][j] == CC.label) {
                    CCAry[i][j] = CC.label;
                } else {
                    CCAry[i][j] = 0;
                }
            }
        }
    }

    void getChainCode(ofstream& chainCodeFile, ofstream& debugFile) {
        debugFile << "entering getChainCode method" << endl;
        chainCodeFile << numRows << " " << numCols << " " << minVal << " " << maxVal << endl;
        int label = CC.label;

        for (int i = 1; i <= numRows; i++) {
            for (int j = 1; j <= numCols; j++) {
                if (CCAry[i][j] == label) {
                    chainCodeFile << i << " " << j << " " << label << endl;
                    startP.row = i;
                    startP.col = j;
                    currentP.row = i;
                    currentP.col = j;
                    lastQ = 4;
                }
            }
        }

        int nextQ;

        do {
            nextQ = (lastQ + 1) % 8;

            PchainDir = findNextP(nextQ, debugFile);

            chainCodeFile << PchainDir << " ";

            nextP.row = currentP.row + coordOffset[PchainDir].row;
            nextP.col = currentP.col + coordOffset[PchainDir].col;
            currentP = nextP;

            lastQ = PchainDir == 0 ? zeroTable[7] : zeroTable[PchainDir - 1];

            debugFile << "lastQ = " << lastQ << "; nextQ = " << nextQ << "; currentP.row = " << currentP.row
                      << "; currentP.col = " << currentP.col << "; nextP.row = " << nextP.row
                      << "; nextP.col = " << nextP.col << endl;

        } while (!(currentP.row == startP.row && currentP.col == startP.col));

        // Step 10: Debug exit message
        debugFile << "Leaving getChainCode" << endl;

    }

    int findNextP(int lastQ, ofstream& debugFile) {
        debugFile << "entering findNextP method" << endl;
        int index = lastQ;
        bool found = false;
        int chainDir;
        do {
            int row = currentP.row + coordOffset[index].row;
            int col = currentP.col + coordOffset[index].col;

            if(imgAry[row][col] == CC.label){
                chainDir = index;
                found = true;
            } else {
                index = (index + 1) % 8;
            }
        } while (!found);

        debugFile << "leaving findNextP method" << endl;
        debugFile << "chainDir = " << chainDir << endl;
        return chainDir;
    }

    void constructBoundary(ifstream& chainCodeFile) {
        string header;
        getline(chainCodeFile, header);  // Skip the header

        int startRow, startCol, label;
        while (chainCodeFile >> startRow >> startCol >> label) {
            chainCodeFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear to end of line

            if (startRow < 1 || startRow > numRows || startCol < 1 || startCol > numCols) {
                cerr << "Invalid start position: startRow = " << startRow << ", startCol = " << startCol << endl;
                continue;
            }

            for (int i = 0; i < numRows + 2; i++) {
                fill(boundaryAry[i], boundaryAry[i] + numCols + 2, 0);
            }

            boundaryAry[startRow][startCol] = label;

            point current = {startRow, startCol};

            string directions;
            getline(chainCodeFile, directions);
            stringstream ss(directions);

            int dir;
            while (ss >> dir) {
                current.row += coordOffset[dir].row;
                current.col += coordOffset[dir].col;

                // Check bounds before marking boundary point
                if (current.row >= 1 && current.row <= numRows && current.col >= 1 && current.col <= numCols) {
                    boundaryAry[current.row][current.col] = label;
                } else {
                    cerr << "Out of bounds access during chain traversal: row = " << current.row << ", col = " << current.col << endl;
                }
            }
        }
    }

    void imgReformat(ofstream& boundaryFile){
        boundaryFile << numRows << " " << numCols << " " << minVal << " " << maxVal << endl;
        string str = to_string(maxVal);
        int width = str.length();
        for(int i = 0; i < numRows; i++){
            for(int j = 0; j < numCols; j++){
                if(boundaryAry[i][j] > 0){
                    boundaryFile << boundaryAry[i][j];
                } else {
                    boundaryFile << '0';
                }
                string str2 = to_string(boundaryAry[i][j]);
                int WW = str2.length();
                while (WW <= width){
                    boundaryFile << " ";
                    WW++;
                }
            }
            boundaryFile << endl;
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

    for (int i = 0; i < numCC; i++) {
        propFile >> chainCode.CC.label >> chainCode.CC.numPixels >> chainCode.CC.minRow >> chainCode.CC.minCol
                 >> chainCode.CC.maxRow >> chainCode.CC.maxCol;

        chainCode.clearCCAry();
        chainCode.loadCCAry();

        chainCode.reformatPrettyPrint(chainCode.CCAry, debugFile);

        chainCode.getChainCode(chainCodeFile, debugFile);
    }

    chainCodeFile.close();
    ifstream chainCodeFile2(argv[5]);

    chainCode.constructBoundary(chainCodeFile2);
    chainCode.reformatPrettyPrint(chainCode.boundaryAry, outFile1);
    chainCode.imgReformat(boundaryFile);

    labelFile.close();
    propFile.close();
    outFile1.close();
    debugFile.close();
    chainCodeFile.close();
    boundaryFile.close();
}