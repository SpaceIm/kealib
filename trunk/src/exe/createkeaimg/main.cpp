/*
 *  main.cpp
 *  LibKEA
 *
 *  Created by Pete Bunting on 02/07/2012.
 *  Copyright 2012 LibKEA. All rights reserved.
 *
 *  This file is part of LibKEA.
 *
 *  Permission is hereby granted, free of charge, to any person 
 *  obtaining a copy of this software and associated documentation 
 *  files (the "Software"), to deal in the Software without restriction, 
 *  including without limitation the rights to use, copy, modify, 
 *  merge, publish, distribute, sublicense, and/or sell copies of the 
 *  Software, and to permit persons to whom the Software is furnished 
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be 
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
 *  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 *  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <iostream>
#include <sstream>
#include <string>

#include "H5Cpp.h"

#include "libkea/KEACommon.h"
#include "libkea/KEAException.h"
#include "libkea/KEAImageIO.h"

int main (int argc, char * const argv[]) 
{
    std::string sFilename;
    if( argc == 2 )
    {
        sFilename = argv[1];
    }
    else
    {
        sFilename = "/Users/pete/Desktop/TestImage.kea";
    }

    try 
    {
        H5::H5File *keaImgFile = libkea::KEAImageIO::createKEAImage(sFilename, libkea::kea_32float, 1000, 2000, 3);
        
        //keaImgFile->close();
        //keaImgFile = libkea::KEAImageIO::openKeaH5RW(sFilename);
        
        
        libkea::KEAImageIO imageIO;
        imageIO.openKEAImageHeader(keaImgFile);
        
                
        float *data = new float[100*200];
        for(unsigned int i = 0; i < 200; ++i)
        {
            //data[i] = i;
            for(unsigned int j = 0; j < 100; ++j)
            {
                data[(i*100)+j] = j; 
            }
        }
        
        std::cout << "Write Image data\n";
        imageIO.writeImageBlock2Band(1, data, 0, 0, 100, 200, 100, 200, libkea::kea_32float);
        imageIO.writeImageBlock2Band(2, data, 100, 200, 100, 200, 100, 200, libkea::kea_32float);
        imageIO.writeImageBlock2Band(3, data, 200, 400, 100, 200, 100, 200, libkea::kea_32float);
        std::cout << "Written Image data\n";
        
        imageIO.setImageMetaData("TEST1", "Hello World 1");
        imageIO.setImageMetaData("TEST2", "Hello World 2");
        imageIO.setImageMetaData("TEST3", "Hello World 3");
        imageIO.setImageMetaData("TEST4", "Hello World 4");
        imageIO.setImageMetaData("TEST5", "Hello World 5");
        imageIO.setImageMetaData("TEST6", "Hello World 6");
        imageIO.setImageMetaData("TEST7", "Hello World 7");
        imageIO.setImageBandMetaData(1, "TEST1", "Hello World 1 on Band");
        imageIO.setImageBandMetaData(1, "TEST2", "Hello World 2 on Band");
        imageIO.setImageBandMetaData(1, "TEST3", "Hello World 3 on Band");
        imageIO.setImageBandMetaData(1, "TEST4", "Hello World 4 on Band");
        imageIO.setImageBandMetaData(1, "TEST5", "Hello World 5 on Band");
        imageIO.setImageBandMetaData(1, "TEST6", "Hello World 6 on Band");
        
        std::cout << "TEST META-DATA Image: " << imageIO.getImageMetaData("TEST1") << std::endl;
        std::cout << "TEST META-DATA Band: " << imageIO.getImageBandMetaData(1, "TEST1") << std::endl;
        std::cout << "DESCRIPTION 1: " << imageIO.getImageBandDescription(1) << std::endl;
        std::cout << "DESCRIPTION 2: " << imageIO.getImageBandDescription(2) << std::endl;
        std::cout << "DESCRIPTION 3: " << imageIO.getImageBandDescription(3) << std::endl;
        
        std::cout << "Number of Bands: " << imageIO.getNumOfImageBands() << std::endl;
        
        std::cout << "Image Data Type: " << imageIO.getImageBandDataType(1) << std::endl;
        std::cout << "Image Data Type: " << getDataTypeAsStr(imageIO.getImageBandDataType(1)) << std::endl;
        
        int noDataVal = 13;
        
        imageIO.setNoDataValue(1, &noDataVal, libkea::kea_32int);
        std::cout << "No Data value being set to " << noDataVal << std::endl;
        
        float noDataValRead = -1;
        imageIO.getNoDataValue(1, &noDataValRead, libkea::kea_32float);
        std::cout << "No Data value read as " << noDataVal << std::endl;
        
        noDataVal = 2300;
        
        imageIO.setNoDataValue(1, &noDataVal, libkea::kea_32int);
        std::cout << "No Data value being set to " << noDataVal << std::endl;
        
        noDataValRead = -1;
        imageIO.getNoDataValue(1, &noDataValRead, libkea::kea_32float);
        std::cout << "No Data value read as " << noDataVal << std::endl;
        
        
        std::vector<std::string> bandMetaDataNames =  imageIO.getImageBandMetaDataNames(1);
        std::cout << "Image Band 1 Meta Data Names: \n";
        for(std::vector<std::string>::iterator iterNames = bandMetaDataNames.begin(); iterNames != bandMetaDataNames.end(); ++iterNames)
        {
            std::cout << "\t" << *iterNames << std::endl;
        }
        
        std::vector<std::string> imgMetaDataNames =  imageIO.getImageMetaDataNames();
        std::cout << "Image Meta Data Names: \n";
        for(std::vector<std::string>::iterator iterNames = imgMetaDataNames.begin(); iterNames != imgMetaDataNames.end(); ++iterNames)
        {
            std::cout << "\t" << *iterNames << std::endl;
        }
        
        std::vector< std::pair<std::string, std::string> > imgBandMetaData = imageIO.getImageBandMetaData(1);
        std::cout << "Image Band Meta Data: \n";
        for(std::vector< std::pair<std::string, std::string> >::iterator iterMetaData = imgBandMetaData.begin(); iterMetaData != imgBandMetaData.end(); ++iterMetaData)
        {
            std::cout << "\t" << iterMetaData->first << " = " << iterMetaData->second << std::endl;
        }
        
        std::vector< std::pair<std::string, std::string> > imgMetaData = imageIO.getImageMetaData();
        std::cout << "Image Meta Data: \n";
        for(std::vector< std::pair<std::string, std::string> >::iterator iterMetaData = imgMetaData.begin(); iterMetaData != imgMetaData.end(); ++iterMetaData)
        {
            std::cout << "\t" << iterMetaData->first << " = " << iterMetaData->second << std::endl;
        }
        
        std::vector< std::pair<std::string, std::string> > newMetaData;
        
        newMetaData.push_back(std::pair<std::string, std::string>("min1", "4"));
        newMetaData.push_back(std::pair<std::string, std::string>("max1", "4543"));
        newMetaData.push_back(std::pair<std::string, std::string>("min2", "45"));
        newMetaData.push_back(std::pair<std::string, std::string>("max2", "67546"));
        newMetaData.push_back(std::pair<std::string, std::string>("min3", "12"));
        newMetaData.push_back(std::pair<std::string, std::string>("max3", "900"));
        
        imageIO.setImageBandMetaData(2, newMetaData);
        imageIO.setImageMetaData(newMetaData);
        
        std::cout << "Creating overview images\n";
        imageIO.createOverview(1, 0, 500, 1000);
        imageIO.createOverview(1, 1, 250, 500);
        imageIO.createOverview(1, 1, 500, 500);
        
        imageIO.createOverview(2, 0, 500, 1000);
        imageIO.createOverview(2, 1, 250, 500);
        
        imageIO.createOverview(3, 0, 500, 1000);
        imageIO.createOverview(3, 1, 250, 500);
        
        std::cout << "Overview 1 Band 3 block size: " << imageIO.getOverviewBlockSize(3, 1) << std::endl;
        
        std::cout << "There are " << imageIO.getNumOfOverviews(1) << " overviews for band 1\n";
        
        unsigned long xSizeOverview = 0;
        unsigned long ySizeOverview = 0;
        
        imageIO.getOverviewSize(2, 1, &xSizeOverview, &ySizeOverview);
        
        std::cout << "Image overview 1 for band 2 has size [" << xSizeOverview << "," << ySizeOverview << "]\n";
        
        for(unsigned int i = 0; i < (100*200); ++i)
        {
            data[i] = 0;
        }
        
        imageIO.readImageBlock2Band(1, data, 50, 100, 100, 200, 100, 200, libkea::kea_32float);
        
        /*for(unsigned int i = 0; i < 200; ++i)
        {
            for(unsigned int j = 0; j < 100; ++j)
            {
                if(j == 0)
                {
                    std::cout << data[(i*100)+j];
                }
                else
                {
                    std::cout << "," << data[(i*100)+j];
                }
            }
            std::cout << std::endl;
        }*/
        
        delete[] data;
        
        imageIO.close();
        
        keaImgFile->close();
        
        if(libkea::KEAImageIO::isKEAImage("/Users/pete/Desktop/TestImage.kea"))
        {
            std::cout << "FOUND KEA IMAGE - THIS IS CORRECT A KEA IMAGE WAS PROVIDED!\n";
        }
        else
        {
            std::cout << "DID NOT FIND KEA IMAGE - THIS IS **NOT** CORRECT A KEA IMAGE WAS PROVIDED!\n";
        }
        
        
        if(libkea::KEAImageIO::isKEAImage("/Users/pete/Desktop/LI080204_RAW9_1m_pmfgrd_dtm.img"))
        {
            std::cout << "FOUND KEA IMAGE - THIS IS **NOT** CORRECT A IMG IMAGE WAS PROVIDED!\n";
        }
        else
        {
            std::cout << "DID NOT FIND KEA IMAGE - THIS IS CORRECT A IMG IMAGE WAS PROVIDED!\n";
        }
        
        if(libkea::KEAImageIO::isKEAImage("/Users/pete/Desktop/LI080220_RAW17_10m_pmfgrd_h.spd"))
        {
            std::cout << "FOUND KEA IMAGE - THIS IS **NOT** CORRECT A SPD IMAGE WAS PROVIDED!\n";
        }
        else
        {
            std::cout << "DID NOT FIND KEA IMAGE - THIS IS CORRECT A SPD IMAGE WAS PROVIDED!\n";
        }
    } 
    catch (libkea::KEAException &e) 
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    
}




