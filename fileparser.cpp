#include "fileparser.h"

fileParser::fileParser(QObject *parent) :
    QObject(parent)
{
}

void fileParser::clearList()
{
    this->desiredFiles.clear();
}

double fileParser::MSA_calculator(QString data, int &modelNum)
{
    int neighbors = 5;
    QStringList lines = data.split("\n");
    QChar modelNum_char = lines.at(1).at(9);
    modelNum = modelNum_char.digitValue();
    int correctEdges = 0, allEdges = 0;
    for(int i = 4; i < lines.size()-1; i++)
    {
        QString binaryCode = lines.at(i).simplified();
        binaryCode.replace(" ","");
        for(int j = 0; j < binaryCode.size(); j++)
        {
            if(binaryCode.at(j) == '1')
            {
                allEdges++;
                if((i-j-4)<neighbors)
                    correctEdges++;
            }
        }
    }

    if( allEdges == 0 )
        return -1;

    double output = (double) correctEdges / (double) allEdges;
    return output;
}

void fileParser::sepratingFiles(const QString &parameter)
{
    QDir path(parameter);
    QDirIterator it(parameter, QDirIterator::Subdirectories);
    int numberOfFiles = 0;

    while (it.hasNext()) {
        numberOfFiles++;
        QString fileName = it.next();
        if( fileName.endsWith(".network") )
        {
            qDebug() << fileName;
            int percent = double(numberOfFiles/path.count())*100;
            emit progressResult(fileName,percent);
            desiredFiles.append(fileName);
        }
    }
    emit fetching_finished();
}

void fileParser::processingFiles()
{
    emit processingResult("Processing Starts...",0);
    for(int i = 0; i < desiredFiles.size(); i++)
    {
        QStringList list1 = desiredFiles.at(i).split("/");
        QString tmp = "File '";
        tmp.append(list1.last());
        tmp.append("' is processing...");
        int percent = ((double)i/desiredFiles.length())*100;
        emit processingResult(tmp,percent);

        int numberOfModels;
        QString y = "MultiModel";
        int id = desiredFiles.at(i).indexOf(y);
        QChar numberOfModels_char;
        if( desiredFiles.at(i).at(id+y.size()) >= '1' && desiredFiles.at(i).at(id+y.size()) <= '9')
            numberOfModels_char = desiredFiles.at(i).at(id+y.size());
        else
            numberOfModels_char = desiredFiles.at(i).at(id+y.size()+1);

        numberOfModels = numberOfModels_char.digitValue();

        QFile file(desiredFiles.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString errorText = "File '";
            errorText.append(list1.last());
            errorText.append("' is not available");
            emit processingResult(errorText,percent);
            continue;
        }

        QTextStream in(&file);
        QString data = in.readAll();
        QStringList runParts = data.split("\nrun number");
        int runNumber = runParts.size() - 1;

        double** MSAs = (double**)calloc(runNumber,sizeof(double*));
        for(int j = 0; j < runNumber+1; j++)
            MSAs[j] = (double*)calloc(numberOfModels,sizeof(double));
        QList<msa_model> MSAs_inGenerations;

        for(int j = 0; j < runNumber+1; j++)
            for(int k = 0; k < numberOfModels; k++)
                MSAs[j][k] = 0;

        QString outputName = "MSAPerRuns_";
        QStringList outputNameLists = list1.last().split("-");
        for(int n = 0; n < outputNameLists.size(); n++)
        {
            outputName.append(outputNameLists.at(n));
            if( n != outputNameLists.size()-1 ) outputName.append("_");
        }
        outputName.remove(".network");
        outputName.append(".m");

        QFile output_file(outputName);
        if (!output_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QString errorText = "File '";
            errorText.append(outputName);
            errorText.append("' cannot be created");
            emit processingResult(errorText,percent);
            continue;
        }

        QTextStream out(&output_file);
        out << "clc\n";
        out << "clear all\n\n";

        for(int runID = 1; runID < runParts.size();runID++)
        {
            MSAs_inGenerations.clear();
            //Calculate MSA in each bisection run
            QStringList models = runParts.at(runID).split("--------------------------------------------------------\n");
            for(int modelID = 1; modelID < models.size(); modelID++)
            {
                int modelNumber;
                double msa = MSA_calculator(models.at(modelID),modelNumber);
                if( msa != -1)
                {
                    MSAs[runID][modelNumber-1] = msa;
                    msa_model mm;
                    mm.modelNumber = modelNumber;
                    mm.MSA = msa;
                    MSAs_inGenerations.append((mm));
                }
            }
            for(int m = 1; m < numberOfModels+1; m++)
            {
                out <<"\nrun"<<runID<<"_model"<<m<<"=[";
                for(int n = 0; n < MSAs_inGenerations.size();n++)
                {
                    if(MSAs_inGenerations.at(n).modelNumber == m )
                        out<<" "<<MSAs_inGenerations.at(n).MSA;

                    if(n == MSAs_inGenerations.size()-1)
                        out<<"];\n";
                }
            }
            qDebug()<<"runID: "<<runID<<"finished";
        }

        QStringList alphabets;
        alphabets<<"A"<<"B"<<"C"<<"D"<<"E"<<"F"<<"G"<<"H"<<"I"<<"J";

        for(int j = 0; j < numberOfModels; j++)
        {
            out <<alphabets.at(j)<<"=[";
            for(int k = 1; k < runNumber+1; k++)
            {
                out<< MSAs[k][j];
                if( k == runNumber )
                    out<<"];\n";
                else
                    out<<" ";
            }
        }
        out << "runNumber = 1:"<<runNumber<<";\n";
        out << "figure();\n";
        for(int j = 0; j < numberOfModels; j++)
        {
            out<<"subplot(1,"<<numberOfModels<<","<<j+1<<");\n";
            out<<"plot(runNumber,"<<alphabets.at(j)<<");\n";
            out<<"title('Model"<<j+1<<"');\n";
        }
        QString outputLog = outputName;
        outputLog.append(" is created.");
        emit processingResult(outputLog ,percent);

        output_file.close();
        file.close();

        for(int j = 0; j < runNumber+1; j++)
            free(MSAs[j]);
        free(MSAs);
    }
    emit processing_finished();
}
