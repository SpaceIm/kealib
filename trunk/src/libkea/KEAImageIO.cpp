/*
 *  KEAImageIO.cpp
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

#include "libkea/KEAImageIO.h"

namespace libkea{
    
    KEAImageIO::KEAImageIO()
    {
        this->fileOpen = false;
    }
    
    void KEAImageIO::openKEAImageHeader(H5::H5File *keaImgH5File)throw(KEAIOException)
    {
        try 
        {
            this->keaImgFile = keaImgH5File;
            this->spatialInfoFile = new KEAImageSpatialInfo();
            
            // READ NUMBER OF IMAGE BANDS
            try 
            {
                hsize_t dimsValue[1];
                dimsValue[0] = 1;
                H5::DataSpace valueDataSpace(1, dimsValue);
                unsigned int value[1];
                H5::DataSet datasetNumImgBands = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_NUMBANDS );
                datasetNumImgBands.read(value, H5::PredType::NATIVE_UINT, valueDataSpace);
                this->numImgBands = value[0];
                datasetNumImgBands.close();
                valueDataSpace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The number of image bands was not specified.");
            }
                        
            // READ TL COORDINATES
            try 
            {
                hsize_t dimsValue[1];
                dimsValue[0] = 2;
                H5::DataSpace valueDataSpace(1, dimsValue);
                double *values = new double[2];
                H5::DataSet datasetSpatialTL = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_TL );
                datasetSpatialTL.read(values, H5::PredType::NATIVE_DOUBLE, valueDataSpace);
                this->spatialInfoFile->tlX = values[0];
                this->spatialInfoFile->tlY = values[1];
                datasetSpatialTL.close();
                valueDataSpace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The TL coordinate is not specified.");
            }
            
            // READ RESOLUTION
            try 
            {
                hsize_t dimsValue[1];
                dimsValue[0] = 2;
                H5::DataSpace valueDataSpace(1, dimsValue);
                float *values = new float[2];
                H5::DataSet spatialResDataset = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_RES );
                spatialResDataset.read(values, H5::PredType::NATIVE_FLOAT, valueDataSpace);
                this->spatialInfoFile->xRes = values[0];
                this->spatialInfoFile->yRes = values[1];
                spatialResDataset.close();
                valueDataSpace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The pixel resolution was not specified.");
            }
            
            // READ ROTATION
            try 
            {
                hsize_t dimsValue[1];
                dimsValue[0] = 2;
                H5::DataSpace valueDataSpace(1, dimsValue);
                float *values = new float[2];
                H5::DataSet spatialRotDataset = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_ROT );
                spatialRotDataset.read(values, H5::PredType::NATIVE_FLOAT, valueDataSpace);
                this->spatialInfoFile->xRot = values[0];
                this->spatialInfoFile->yRot = values[1];
                spatialRotDataset.close();
                valueDataSpace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The image resolution was not specified.");
            }
            
            // READ IMAGE SIZE
            try 
            {
                hsize_t dimsValue[1];
                dimsValue[0] = 2;
                H5::DataSpace valueDataSpace(1, dimsValue);
                unsigned long *values = new unsigned long[2];
                H5::DataSet spatialSizeDataset = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_SIZE );
                spatialSizeDataset.read(values, H5::PredType::NATIVE_ULONG, valueDataSpace);
                this->spatialInfoFile->xSize = values[0];
                this->spatialInfoFile->ySize = values[1];
                spatialSizeDataset.close();
                valueDataSpace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The image resolution was not specified.");
            }
            
            // READ WKT STRING
            try 
            {
                hid_t nativeStrType;
                H5::DataSet datasetSpatialReference = this->keaImgFile->openDataSet( KEA_DATASETNAME_HEADER_WKT );
                H5::DataType strDataType = datasetSpatialReference.getDataType();
                char **strData = new char*[1];
                if((nativeStrType=H5Tget_native_type(strDataType.getId(), H5T_DIR_DEFAULT))<0)
                {
                    throw KEAIOException("Could not define a native string type");
                }
                datasetSpatialReference.read((void*)strData, strDataType);
                this->spatialInfoFile->wktString = std::string(strData[0]);
                delete strData[0];
                delete[] strData;
                datasetSpatialReference.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("The spatial reference was not specified.");
            }
        } 
        catch (KEAIOException &e) 
        {
            throw e;
        }
        
        this->fileOpen = true;
    }
    
    void KEAImageIO::writeImageBlock2Band(unsigned int band, void *data, unsigned long xPxlOff, unsigned long yPxlOff, unsigned long xSizeOut, unsigned long ySizeOut, unsigned long xSizeBuf, unsigned long ySizeBuf, KEADataType inDataType)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
               throw KEAIOException("Band is not present within image."); 
            }
            
            unsigned long endXPxl = xPxlOff + xSizeOut;
            unsigned long endYPxl = yPxlOff + ySizeOut;
            
            if(xPxlOff > this->spatialInfoFile->xSize)
            {
               throw KEAIOException("Start X Pixel is not within image.");  
            }
            
            if(endXPxl > this->spatialInfoFile->xSize)
            {
                throw KEAIOException("End X Pixel is not within image.");  
            }
            
            if(yPxlOff > this->spatialInfoFile->ySize)
            {
                throw KEAIOException("Start Y Pixel is not within image.");  
            }
            
            if(endYPxl > this->spatialInfoFile->ySize)
            {
                throw KEAIOException("End Y Pixel is not within image.");  
            }
            
            // GET NATIVE DATASET
            H5::DataType imgBandDT = H5::PredType::NATIVE_FLOAT;
            if(inDataType == kea_8int)
            {
                imgBandDT = H5::PredType::NATIVE_INT8;
            }
            else if(inDataType == kea_16int)
            {
                imgBandDT = H5::PredType::NATIVE_INT16;
            }
            else if(inDataType == kea_32int)
            {
                imgBandDT = H5::PredType::NATIVE_INT32;
            }
            else if(inDataType == kea_64int)
            {
                imgBandDT = H5::PredType::NATIVE_INT64;
            }
            else if(inDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT8;
            }
            else if(inDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT16;
            }
            else if(inDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT32;
            }
            else if(inDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT64;
            }
            else if(inDataType == kea_32float)
            {
                imgBandDT = H5::PredType::NATIVE_FLOAT;
            }
            else if(inDataType == kea_64float)
            {
                imgBandDT = H5::PredType::NATIVE_DOUBLE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            // OPEN BAND DATASET AND WRITE IMAGE DATA
            try 
            {
                std::string imageBandPath = KEA_DATASETNAME_BAND + uint2Str(band);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( imageBandPath + KEA_BANDNAME_DATA );
                H5::DataSpace imgBandDataspace = imgBandDataset.getSpace();                
                
                hsize_t imgOffset[2];
                imgOffset[0] = yPxlOff;
                imgOffset[1] = xPxlOff;
                hsize_t dataDims[2];
                dataDims[0] = ySizeBuf;
                dataDims[1] = xSizeBuf;
                H5::DataSpace write2BandDataspace = H5::DataSpace(2, dataDims);
                
                if((ySizeOut != ySizeBuf) | (xSizeOut != xSizeBuf))
                {                    
                    hsize_t dataSelectMemDims[2];
                    dataSelectMemDims[0] = ySizeOut;
                    dataSelectMemDims[1] = 1;

                    hsize_t dataOffDims[2];
                    dataOffDims[0] = 0;
                    dataOffDims[1] = 0;

                    hsize_t dataSelectStrideDims[2];
                    dataSelectStrideDims[0] = 1;
                    if(xSizeBuf == xSizeOut)
                    {
                        dataSelectStrideDims[1] = 1;
                    }
                    else
                    {
                        dataSelectStrideDims[1] = xSizeBuf - xSizeOut;
                    }

                    hsize_t dataSelectBlockSizeDims[2];
                    dataSelectBlockSizeDims[0] = 1;
                    dataSelectBlockSizeDims[1] = xSizeOut;
                    write2BandDataspace.selectHyperslab(H5S_SELECT_SET, dataSelectMemDims, dataOffDims, dataSelectStrideDims, dataSelectBlockSizeDims);
                    
                    hsize_t dataOutDims[2];
                    dataOutDims[0] = ySizeOut;
                    dataOutDims[1] = xSizeOut;
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataOutDims, imgOffset);
                }
                else
                {
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataDims, imgOffset);
                }
                
                imgBandDataset.write( data, imgBandDT, write2BandDataspace, imgBandDataspace);
                                
                imgBandDataset.close();
                imgBandDataspace.close();
                write2BandDataspace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not write image data.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
    }
    
    void KEAImageIO::readImageBlock2Band(unsigned int band, void *data, unsigned long xPxlOff, unsigned long yPxlOff, unsigned long xSizeIn, unsigned long ySizeIn, unsigned long xSizeBuf, unsigned long ySizeBuf, KEADataType inDataType)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
            
            unsigned long endXPxl = xPxlOff + xSizeIn;
            unsigned long endYPxl = yPxlOff + ySizeIn;
            
            if(xPxlOff > this->spatialInfoFile->xSize)
            {
                throw KEAIOException("Start X Pixel is not within image.");  
            }
            
            if(endXPxl > this->spatialInfoFile->xSize)
            {
                throw KEAIOException("End X Pixel is not within image.");  
            }
            
            if(yPxlOff > this->spatialInfoFile->ySize)
            {
                throw KEAIOException("Start Y Pixel is not within image.");  
            }
            
            if(endYPxl > this->spatialInfoFile->ySize)
            {
                throw KEAIOException("End Y Pixel is not within image.");  
            }
            
            // GET NATIVE DATASET
            H5::DataType imgBandDT = H5::PredType::NATIVE_FLOAT;
            if(inDataType == kea_8int)
            {
                imgBandDT = H5::PredType::NATIVE_INT8;
            }
            else if(inDataType == kea_16int)
            {
                imgBandDT = H5::PredType::NATIVE_INT16;
            }
            else if(inDataType == kea_32int)
            {
                imgBandDT = H5::PredType::NATIVE_INT32;
            }
            else if(inDataType == kea_64int)
            {
                imgBandDT = H5::PredType::NATIVE_INT64;
            }
            else if(inDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT8;
            }
            else if(inDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT16;
            }
            else if(inDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT32;
            }
            else if(inDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT64;
            }
            else if(inDataType == kea_32float)
            {
                imgBandDT = H5::PredType::NATIVE_FLOAT;
            }
            else if(inDataType == kea_64float)
            {
                imgBandDT = H5::PredType::NATIVE_DOUBLE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            // OPEN BAND DATASET AND READ IMAGE DATA
            try 
            {
                std::string imageBandPath = KEA_DATASETNAME_BAND + uint2Str(band);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( imageBandPath + KEA_BANDNAME_DATA );
                H5::DataSpace imgBandDataspace = imgBandDataset.getSpace();                
                
                hsize_t dataOffset[2];
                dataOffset[0] = yPxlOff;
                dataOffset[1] = xPxlOff;
                hsize_t dataDims[2];
                dataDims[0] = ySizeBuf;
                dataDims[1] = xSizeBuf;
                H5::DataSpace read2BandDataspace = H5::DataSpace(2, dataDims);
                
                if((ySizeBuf != ySizeIn) | (xSizeBuf != xSizeIn))
                {
                    hsize_t dataSelectMemDims[2];
                    dataSelectMemDims[0] = ySizeIn;
                    dataSelectMemDims[1] = 1;

                    hsize_t dataOffDims[2];
                    dataOffDims[0] = 0;
                    dataOffDims[1] = 0;

                    hsize_t dataSelectStrideDims[2];
                    dataSelectStrideDims[0] = 1;
                    if(xSizeBuf == xSizeIn)
                    {
                        dataSelectStrideDims[1] = 1;
                    }
                    else
                    {
                        dataSelectStrideDims[1] = xSizeBuf - xSizeIn;
                    }

                    hsize_t dataSelectBlockSizeDims[2];
                    dataSelectBlockSizeDims[0] = 1;
                    dataSelectBlockSizeDims[1] = xSizeIn;
                    read2BandDataspace.selectHyperslab(H5S_SELECT_SET, dataSelectMemDims, dataOffDims, dataSelectStrideDims, dataSelectBlockSizeDims);
                    
                    hsize_t dataInDims[2];
                    dataInDims[0] = ySizeIn;
                    dataInDims[1] = xSizeIn;
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataInDims, dataOffset);
                }
                else
                {
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataDims, dataOffset);
                }
                
                imgBandDataset.read( data, imgBandDT, read2BandDataspace, imgBandDataspace);
                
                imgBandDataset.close();
                imgBandDataspace.close();
                read2BandDataspace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not read image data.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
    }
    
    void KEAImageIO::setImageMetaData(std::string name, std::string value)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        // FORM META-DATA PATH WITHIN THE H5 FILE 
        std::string metaDataH5Path = KEA_DATASETNAME_METADATA + std::string("/") + name;
        
        // WRITE IMAGE META DATA
        try 
        {
            // OPEN DATASET OR CREATE NEW DATASET IF IT DOES NOT EXIST
            H5::StrType strTypeAll(0, H5T_VARIABLE);
            H5::DataSet datasetMetaData;
            try 
            {
                datasetMetaData = this->keaImgFile->openDataSet( metaDataH5Path );
            }
            catch (H5::Exception &e)
            {
                hsize_t	dimsForStr[1];
                dimsForStr[0] = 1; // number of lines;
                H5::DataSpace dataspaceStrAll(1, dimsForStr);
                datasetMetaData = this->keaImgFile->createDataSet(metaDataH5Path, strTypeAll, dataspaceStrAll);

            }
            // WRITE DATA INTO THE DATASET
            const char **wStrdata = new const char*[1];
            wStrdata[0] = value.c_str();			
            datasetMetaData.write((void*)wStrdata, strTypeAll);
            datasetMetaData.close();
            delete[] wStrdata;
        }
        catch (H5::Exception &e) 
        {
            throw KEAIOException("Could not set image meta-data.");
        }
    }
    
    std::string KEAImageIO::getImageMetaData(std::string name)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string metaDataH5Path = KEA_DATASETNAME_METADATA + std::string("/") + name;
        std::string value = "";
        // READ IMAGE META-DATA
        try 
        {
            hid_t nativeStrType;
            H5::DataSet datasetMetaData = this->keaImgFile->openDataSet( metaDataH5Path );
            H5::DataType strDataType = datasetMetaData.getDataType();
            char **strData = new char*[1];
            if((nativeStrType=H5Tget_native_type(strDataType.getId(), H5T_DIR_DEFAULT))<0)
            {
                throw KEAIOException("Could not define a native string type");
            }
            datasetMetaData.read((void*)strData, strDataType);
            value = std::string(strData[0]);
            delete strData[0];
            delete[] strData;
            datasetMetaData.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("Meta-data variable was not accessable.");
        }
        
        return value;
    }
    
    std::vector<std::string> KEAImageIO::getImageMetaDataNames()throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::vector<std::string> metaDataNames;
        
        try 
        {
            // Try to open dataset with overviewName
            H5::Group imgBandMetaDataGrp = this->keaImgFile->openGroup(KEA_DATASETNAME_METADATA);
            hsize_t numMetaDataItems = imgBandMetaDataGrp.getNumObjs();
            
            for(hsize_t i = 0; i < numMetaDataItems; ++i)
            {
                metaDataNames.push_back(imgBandMetaDataGrp.getObjnameByIdx(i));
            }
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not retrieve image meta data.");
        }
        
        return metaDataNames;
    }
    
    std::vector< std::pair<std::string, std::string> > KEAImageIO::getImageMetaData()throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::vector< std::pair<std::string, std::string> > metaData;
        
        try 
        {
            // Try to open dataset with overviewName
            H5::Group imgBandMetaDataGrp = this->keaImgFile->openGroup(KEA_DATASETNAME_METADATA);
            hsize_t numMetaDataItems = imgBandMetaDataGrp.getNumObjs();
            std::string name = "";
            std::string value = "";
            
            for(hsize_t i = 0; i < numMetaDataItems; ++i)
            {
                name = imgBandMetaDataGrp.getObjnameByIdx(i);
                value = this->getImageMetaData(name);
                metaData.push_back(std::pair<std::string, std::string>(name,value));
            }
        }
        catch (KEAIOException &e)
        {
            throw e;
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not retrieve image meta data.");
        }
        
        return metaData;
    }
    
    void KEAImageIO::setImageMetaData(std::vector< std::pair<std::string, std::string> > data)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            for(std::vector< std::pair<std::string, std::string> >::iterator iterMetaData = data.begin(); iterMetaData != data.end(); ++iterMetaData)
            {
                this->setImageMetaData(iterMetaData->first, iterMetaData->second);
            }
        }
        catch (KEAIOException &e)
        {
            throw e;
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not set image band meta data.");
        }
    }
    
    void KEAImageIO::setImageBandMetaData(unsigned int band, std::string name, std::string value)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        // FORM META-DATA PATH WITHIN THE H5 FILE 
        std::string metaDataH5Path = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_METADATA + std::string("/") + name;
        
        // WRITE IMAGE BAND META DATA
        try 
        {
            // OPEN DATASET OR CREATE NEW DATASET IF IT DOES NOT EXIST
            H5::StrType strTypeAll(0, H5T_VARIABLE);
            H5::DataSet datasetMetaData;
            try 
            {
                datasetMetaData = this->keaImgFile->openDataSet( metaDataH5Path );
            }
            catch (H5::Exception &e)
            {
                hsize_t	dimsForStr[1];
                dimsForStr[0] = 1; // number of lines;
                H5::DataSpace dataspaceStrAll(1, dimsForStr);
                datasetMetaData = this->keaImgFile->createDataSet(metaDataH5Path, strTypeAll, dataspaceStrAll);
                
            }
            // WRITE DATA INTO THE DATASET
            const char **wStrdata = new const char*[1];
            wStrdata[0] = value.c_str();			
            datasetMetaData.write((void*)wStrdata, strTypeAll);
            datasetMetaData.close();
            delete[] wStrdata;
        }
        catch (H5::Exception &e) 
        {
            throw KEAIOException("Could not set image band meta-data.");
        }
    }
    
    std::string KEAImageIO::getImageBandMetaData(unsigned int band, std::string name)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string metaDataH5Path = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_METADATA + std::string("/") + name;
        std::string value = "";
        // READ IMAGE BAND META-DATA
        try 
        {
            hid_t nativeStrType;
            H5::DataSet datasetMetaData = this->keaImgFile->openDataSet( metaDataH5Path );
            H5::DataType strDataType = datasetMetaData.getDataType();
            char **strData = new char*[1];
            if((nativeStrType=H5Tget_native_type(strDataType.getId(), H5T_DIR_DEFAULT))<0)
            {
                throw KEAIOException("Could not define a native string type");
            }
            datasetMetaData.read((void*)strData, strDataType);
            value = std::string(strData[0]);
            delete strData[0];
            delete[] strData;
            datasetMetaData.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("Meta-data variable was not accessable.");
        }
        
        return value;
    }
    
    std::vector<std::string> KEAImageIO::getImageBandMetaDataNames(unsigned int band)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::vector<std::string> metaDataNames;
        
        std::string metaDataGroupName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_METADATA;
        try 
        {
            // Try to open dataset with overviewName
            H5::Group imgBandMetaDataGrp = this->keaImgFile->openGroup(metaDataGroupName);
            hsize_t numMetaDataItems = imgBandMetaDataGrp.getNumObjs();
            
            for(hsize_t i = 0; i < numMetaDataItems; ++i)
            {
                metaDataNames.push_back(imgBandMetaDataGrp.getObjnameByIdx(i));
            }
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not retrieve image band meta data.");
        }
        
        return metaDataNames;
    }
    
    std::vector< std::pair<std::string, std::string> > KEAImageIO::getImageBandMetaData(unsigned int band)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::vector< std::pair<std::string, std::string> > metaData;
        
        std::string metaDataGroupName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_METADATA;
        try 
        {
            // Try to open dataset with overviewName
            H5::Group imgBandMetaDataGrp = this->keaImgFile->openGroup(metaDataGroupName);
            hsize_t numMetaDataItems = imgBandMetaDataGrp.getNumObjs();
            std::string name = "";
            std::string value = "";
            
            for(hsize_t i = 0; i < numMetaDataItems; ++i)
            {
                name = imgBandMetaDataGrp.getObjnameByIdx(i);
                value = this->getImageBandMetaData(band, name);
                metaData.push_back(std::pair<std::string, std::string>(name,value));
            }
        }
        catch (KEAIOException &e)
        {
            throw e;
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not retrieve image band meta data.");
        }
        
        return metaData;
    }
    
    void KEAImageIO::setImageBandMetaData(unsigned int band, std::vector< std::pair<std::string, std::string> > data)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            for(std::vector< std::pair<std::string, std::string> >::iterator iterMetaData = data.begin(); iterMetaData != data.end(); ++iterMetaData)
            {
                this->setImageBandMetaData(band, iterMetaData->first, iterMetaData->second);
            }
        }
        catch (KEAIOException &e)
        {
            throw e;
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not set image band meta data.");
        }
    }
    
    void KEAImageIO::setImageBandDescription(unsigned int band, std::string description)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string bandName = KEA_DATASETNAME_BAND + uint2Str(band);
        
        // WRITE IMAGE BAND DESCRIPTION
        try 
        {
            H5::StrType strTypeAll(0, H5T_VARIABLE);
            H5::DataSet datasetBandDescription = this->keaImgFile->openDataSet( bandName+KEA_BANDNAME_DESCRIP );
            const char **wStrdata = new const char*[1];
            wStrdata[0] = description.c_str();			
            datasetBandDescription.write((void*)wStrdata, strTypeAll);
            datasetBandDescription.close();
            delete[] wStrdata;
        }
        catch (H5::Exception &e) 
        {
            throw KEAIOException("Could not set image band description.");
        }
    }
    
    std::string KEAImageIO::getImageBandDescription(unsigned int band)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string bandName = KEA_DATASETNAME_BAND + uint2Str(band) + std::string("/") + KEA_BANDNAME_DESCRIP;
        std::string description = "";
        // READ IMAGE BAND DESCRIPTION
        try 
        {
            hid_t nativeStrType;
            H5::DataSet datasetBandDescription = this->keaImgFile->openDataSet( bandName );
            H5::DataType strDataType = datasetBandDescription.getDataType();
            char **strData = new char*[1];
            if((nativeStrType=H5Tget_native_type(strDataType.getId(), H5T_DIR_DEFAULT))<0)
            {
                throw KEAIOException("Could not define a native string type");
            }
            datasetBandDescription.read((void*)strData, strDataType);
            description = std::string(strData[0]);
            delete strData[0];
            delete[] strData;
            datasetBandDescription.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("Could not read band description.");
        }
        
        return description;
    }
        
    void KEAImageIO::setNoDataValue(unsigned int band, void *data, KEADataType inDataType)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        // READ IMAGE DATA TYPE
        try 
        {    
            std::string noDataValPath = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_NO_DATA_VAL;
            H5::DataSet datasetImgNDV;
            
            try 
            {
                datasetImgNDV = this->keaImgFile->openDataSet( noDataValPath );
            }
            catch (H5::Exception &e)
            {
                hsize_t	dimsForNDV[1];
                dimsForNDV[0] = 1; // number of lines;
                H5::DataSpace dataspaceNDV(1, dimsForNDV);
                
                KEADataType imgDataType = this->getImageBandDataType(band);
                
                H5::DataType imgBandDT = H5::PredType::IEEE_F32LE;
                if(imgDataType == kea_8int)
                {
                    imgBandDT = H5::PredType::STD_I8LE;
                }
                else if(imgDataType == kea_16int)
                {
                    imgBandDT = H5::PredType::STD_I16LE;
                }
                else if(imgDataType == kea_32int)
                {
                    imgBandDT = H5::PredType::STD_I32LE;
                }
                else if(imgDataType == kea_64int)
                {
                    imgBandDT = H5::PredType::STD_I64LE;
                }
                else if(imgDataType == kea_8uint)
                {
                    imgBandDT = H5::PredType::STD_U8LE;
                }
                else if(imgDataType == kea_16uint)
                {
                    imgBandDT = H5::PredType::STD_U16LE;
                }
                else if(imgDataType == kea_32uint)
                {
                    imgBandDT = H5::PredType::STD_U32LE;
                }
                else if(imgDataType == kea_64uint)
                {
                    imgBandDT = H5::PredType::STD_U64LE;
                }
                else if(imgDataType == kea_32float)
                {
                    imgBandDT = H5::PredType::IEEE_F32LE;
                }
                else if(imgDataType == kea_64float)
                {
                    imgBandDT = H5::PredType::IEEE_F64LE;
                }
                else
                {
                    throw KEAIOException("The specified data type was not recognised.");
                }
                
                datasetImgNDV = this->keaImgFile->createDataSet(noDataValPath, imgBandDT, dataspaceNDV);
            }
                        
            H5::DataType dataDT = H5::PredType::IEEE_F32LE;
            if(inDataType == kea_8int)
            {
                dataDT = H5::PredType::STD_I8LE;
            }
            else if(inDataType == kea_16int)
            {
                dataDT = H5::PredType::STD_I16LE;
            }
            else if(inDataType == kea_32int)
            {
                dataDT = H5::PredType::STD_I32LE;
            }
            else if(inDataType == kea_64int)
            {
                dataDT = H5::PredType::STD_I64LE;
            }
            else if(inDataType == kea_8uint)
            {
                dataDT = H5::PredType::STD_U8LE;
            }
            else if(inDataType == kea_16uint)
            {
                dataDT = H5::PredType::STD_U16LE;
            }
            else if(inDataType == kea_32uint)
            {
                dataDT = H5::PredType::STD_U32LE;
            }
            else if(inDataType == kea_64uint)
            {
                dataDT = H5::PredType::STD_U64LE;
            }
            else if(inDataType == kea_32float)
            {
                dataDT = H5::PredType::IEEE_F32LE;
            }
            else if(inDataType == kea_64float)
            {
                dataDT = H5::PredType::IEEE_F64LE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            datasetImgNDV.write( data, dataDT );
            datasetImgNDV.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("The image data type was not specified.");
        }  
    }
    
    void KEAImageIO::getNoDataValue(unsigned int band, void *data, KEADataType inDataType)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        
        // READ IMAGE BAND NO DATA VALUE
        try 
        {            
            H5::DataType imgBandDT = H5::PredType::IEEE_F32LE;
            if(inDataType == kea_8int)
            {
                imgBandDT = H5::PredType::STD_I8LE;
            }
            else if(inDataType == kea_16int)
            {
                imgBandDT = H5::PredType::STD_I16LE;
            }
            else if(inDataType == kea_32int)
            {
                imgBandDT = H5::PredType::STD_I32LE;
            }
            else if(inDataType == kea_64int)
            {
                imgBandDT = H5::PredType::STD_I64LE;
            }
            else if(inDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::STD_U8LE;
            }
            else if(inDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::STD_U16LE;
            }
            else if(inDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::STD_U32LE;
            }
            else if(inDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::STD_U64LE;
            }
            else if(inDataType == kea_32float)
            {
                imgBandDT = H5::PredType::IEEE_F32LE;
            }
            else if(inDataType == kea_64float)
            {
                imgBandDT = H5::PredType::IEEE_F64LE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            hsize_t dimsValue[1];
            dimsValue[0] = 1;
            H5::DataSpace valueDataSpace(1, dimsValue);
            H5::DataSet datasetImgNDV = this->keaImgFile->openDataSet( KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_NO_DATA_VAL );
            datasetImgNDV.read(data, imgBandDT, valueDataSpace);
            datasetImgNDV.close();
            valueDataSpace.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("The image band no data vakue was not specified.");
        }
    }
    
    void KEAImageIO::setSpatialInfo(KEAImageSpatialInfo *inSpatialInfo)throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // SET X AND Y TL IN GLOBAL HEADER
            double *doubleVals = new double[2];
            doubleVals[0] = inSpatialInfo->tlX;
            doubleVals[1] = inSpatialInfo->tlY;
            H5::DataSet spatialTLDataset = this->keaImgFile->openDataSet(KEA_DATASETNAME_HEADER_TL);
			spatialTLDataset.write( doubleVals, H5::PredType::NATIVE_DOUBLE );
            spatialTLDataset.close();
            delete[] doubleVals;
            
            // SET X AND Y RESOLUTION IN GLOBAL HEADER
            float *floatVals = new float[2];
            floatVals[0] = inSpatialInfo->xRes;
            floatVals[1] = inSpatialInfo->yRes;
            H5::DataSet spatialResDataset = this->keaImgFile->openDataSet(KEA_DATASETNAME_HEADER_RES);
			spatialResDataset.write( floatVals, H5::PredType::NATIVE_FLOAT );
            spatialResDataset.close();
            
            // SET X AND Y ROTATION IN GLOBAL HEADER
            floatVals[0] = inSpatialInfo->xRot;
            floatVals[1] = inSpatialInfo->yRot;
            H5::DataSet spatialRotDataset = this->keaImgFile->openDataSet(KEA_DATASETNAME_HEADER_ROT);
			spatialRotDataset.write( floatVals, H5::PredType::NATIVE_FLOAT );
            spatialRotDataset.close();
            delete[] floatVals;
            
            // SET THE WKT STRING SPATAIL REFERENCE IN GLOBAL HEADER
			const char **wStrdata = NULL;
            H5::DataSet datasetSpatialReference = this->keaImgFile->openDataSet(KEA_DATASETNAME_HEADER_WKT);
            H5::DataType strDataType = datasetSpatialReference.getDataType();
			wStrdata = new const char*[1];
			wStrdata[0] = inSpatialInfo->wktString.c_str();			
			datasetSpatialReference.write((void*)wStrdata, strDataType);
			datasetSpatialReference.close();
			delete[] wStrdata;
        } 
        catch (H5::Exception &e)
        {
            throw KEAIOException(e.getCDetailMsg());
        }
    }
    
    KEAImageSpatialInfo* KEAImageIO::getSpatialInfo() throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        return spatialInfoFile;
    }
    
    unsigned int KEAImageIO::getNumOfImageBands() throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        return this->numImgBands;
    }
    
    unsigned int KEAImageIO::getImageBlockSize(unsigned int band) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        unsigned int imgBlockSize = 0;
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
            
            // OPEN BAND DATASET
            try 
            {
                std::string imageBandPath = KEA_DATASETNAME_BAND + uint2Str(band);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( imageBandPath + KEA_BANDNAME_DATA );
                H5::Attribute blockSizeAtt = imgBandDataset.openAttribute(KEA_ATTRIBUTENAME_BLOCK_SIZE);
                blockSizeAtt.read(H5::PredType::NATIVE_UINT, &imgBlockSize);
                imgBandDataset.close();
                blockSizeAtt.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not get image block size.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        
        return imgBlockSize;
    }
    
    KEADataType KEAImageIO::getImageBandDataType(unsigned int band) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        KEADataType imgDataType = libkea::kea_undefined;
        
        // READ IMAGE DATA TYPE
        try 
        {
            hsize_t dimsValue[1];
            dimsValue[0] = 1;
            H5::DataSpace valueDataSpace(1, dimsValue);
            unsigned int value[1];
            H5::DataSet datasetImgDT = this->keaImgFile->openDataSet( KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_DT );
            datasetImgDT.read(value, H5::PredType::NATIVE_UINT, valueDataSpace);
            imgDataType = (KEADataType)value[0];
            datasetImgDT.close();
            valueDataSpace.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("The image band data type was not specified.");
        }
        
        return imgDataType;
    }
    
    void KEAImageIO::setImageBandLayerType(unsigned int band, KEALayerType imgLayerType) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
                
        // WRITE IMAGE LAYER TYPE
        try 
        {
            H5::DataSet datasetImgLT = this->keaImgFile->openDataSet( KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_TYPE );
            datasetImgLT.write(&imgLayerType, H5::PredType::NATIVE_UINT);
            datasetImgLT.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("The image band data type was not specified.");
        }        
    }
    
    KEALayerType KEAImageIO::getImageBandLayerType(unsigned int band) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        KEALayerType imgLayerType = kea_continuous;
        
        // READ IMAGE LAYER TYPE
        try 
        {
            hsize_t dimsValue[1];
            dimsValue[0] = 1;
            H5::DataSpace valueDataSpace(1, dimsValue);
            unsigned int value[1];
            H5::DataSet datasetImgLT = this->keaImgFile->openDataSet( KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_TYPE );
            datasetImgLT.read(value, H5::PredType::NATIVE_UINT, valueDataSpace);
            imgLayerType = (KEALayerType)value[0];
            datasetImgLT.close();
            valueDataSpace.close();
        } 
        catch ( H5::Exception &e) 
        {
            throw KEAIOException("The image band data type was not specified.");
        }
        
        return imgLayerType;
    }
    
    void KEAImageIO::createOverview(unsigned int band, unsigned int overview, unsigned long xSize, unsigned long ySize) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
                
        try 
        {
            // Try to open dataset with overviewName
            H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
            this->keaImgFile->unlink(overviewName);
        }
        catch (H5::Exception &e)
        {
            // Do nothing as dataset does not exist.
        }
        
        try 
        {
            KEADataType imgDataType = this->getImageBandDataType(band);
            
            H5::DataType imgBandDT = H5::PredType::IEEE_F32LE;
            if(imgDataType == kea_8int)
            {
                imgBandDT = H5::PredType::STD_I8LE;
            }
            else if(imgDataType == kea_16int)
            {
                imgBandDT = H5::PredType::STD_I16LE;
            }
            else if(imgDataType == kea_32int)
            {
                imgBandDT = H5::PredType::STD_I32LE;
            }
            else if(imgDataType == kea_64int)
            {
                imgBandDT = H5::PredType::STD_I64LE;
            }
            else if(imgDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::STD_U8LE;
            }
            else if(imgDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::STD_U16LE;
            }
            else if(imgDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::STD_U32LE;
            }
            else if(imgDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::STD_U64LE;
            }
            else if(imgDataType == kea_32float)
            {
                imgBandDT = H5::PredType::IEEE_F32LE;
            }
            else if(imgDataType == kea_64float)
            {
                imgBandDT = H5::PredType::IEEE_F64LE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            int initFillVal = 0;
            
            hsize_t imageBandDims[2];
            imageBandDims[0] = ySize;
            imageBandDims[1] = xSize;
            H5::DataSpace imgBandDataSpace(2, imageBandDims);
            
            hsize_t dimsImageBandChunk[2];
            // Make sure that the chuck size is not bigger than the dataset.
            unsigned int imgBlockSize = this->getImageBlockSize(band);
            unsigned long smallestAxis = 0;
            if(xSize < ySize)
            {
                smallestAxis = xSize;
            }
            else
            {
                smallestAxis = ySize;
            }
            if(smallestAxis < imgBlockSize)
            {
                dimsImageBandChunk[0] = smallestAxis;
                dimsImageBandChunk[1] = smallestAxis;
            }
            else
            {
                dimsImageBandChunk[0] = imgBlockSize;
                dimsImageBandChunk[1] = imgBlockSize;
            }
			
            
            H5::DSetCreatPropList initParamsImgBand;
			initParamsImgBand.setChunk(2, dimsImageBandChunk);			
			initParamsImgBand.setShuffle();
            initParamsImgBand.setDeflate(KEA_DEFLATE);
			initParamsImgBand.setFillValue( H5::PredType::NATIVE_INT, &initFillVal);
            
            H5::StrType strdatatypeLen6(H5::PredType::C_S1, 6);
            H5::StrType strdatatypeLen4(H5::PredType::C_S1, 4);
            const H5std_string strClassVal ("IMAGE");
            const H5std_string strImgVerVal ("1.2");
            H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
                        
            // CREATE THE IMAGE DATA ARRAY
            H5::DataSet imgBandDataSet = this->keaImgFile->createDataSet(overviewName, imgBandDT, imgBandDataSpace, initParamsImgBand);
            
            H5::Attribute classAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_CLASS, strdatatypeLen6, attr_dataspace);
            classAttribute.write(strdatatypeLen6, strClassVal); 
            classAttribute.close();
            
            H5::Attribute imgVerAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_IMAGE_VERSION, strdatatypeLen4, attr_dataspace);
            imgVerAttribute.write(strdatatypeLen4, strImgVerVal);
            imgVerAttribute.close();
            
            H5::Attribute blockSizeAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_BLOCK_SIZE, H5::PredType::STD_U16LE, attr_dataspace);
            blockSizeAttribute.write(H5::PredType::NATIVE_UINT, &dimsImageBandChunk[0]);
            blockSizeAttribute.close();
            
            imgBandDataSet.close();
            
            attr_dataspace.close();
            imgBandDataSet.close();

        }
        catch (H5::Exception &e)
        {
            throw KEAIOException(e.getCDetailMsg());
        }
    }
    
    void KEAImageIO::removeOverview(unsigned int band, unsigned int overview) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
        
        try 
        {
            // Try to open dataset with overviewName
            H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
            this->keaImgFile->unlink(overviewName);
        }
        catch (H5::Exception &e)
        {
            // Do nothing as dataset does not exist.
        }
        
    }
    
    unsigned int KEAImageIO::getOverviewBlockSize(unsigned int band, unsigned int overview) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        unsigned int ovBlockSize = 0;
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
            
            // OPEN BAND DATASET
            try 
            {
                std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
                H5::Attribute blockSizeAtt = imgBandDataset.openAttribute(KEA_ATTRIBUTENAME_BLOCK_SIZE);
                blockSizeAtt.read(H5::PredType::NATIVE_UINT, &ovBlockSize);
                imgBandDataset.close();
                blockSizeAtt.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not retrieve the overview block size.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        
        return ovBlockSize;
    }
    
    void KEAImageIO::writeToOverview(unsigned int band, unsigned int overview, void *data, unsigned long xPxlOff, unsigned long yPxlOff, unsigned long xSizeOut, unsigned long ySizeOut, unsigned long xSizeBuf, unsigned long ySizeBuf, KEADataType inDataType) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
            
            // GET NATIVE DATASET
            H5::DataType imgBandDT = H5::PredType::NATIVE_FLOAT;
            if(inDataType == kea_8int)
            {
                imgBandDT = H5::PredType::NATIVE_INT8;
            }
            else if(inDataType == kea_16int)
            {
                imgBandDT = H5::PredType::NATIVE_INT16;
            }
            else if(inDataType == kea_32int)
            {
                imgBandDT = H5::PredType::NATIVE_INT32;
            }
            else if(inDataType == kea_64int)
            {
                imgBandDT = H5::PredType::NATIVE_INT64;
            }
            else if(inDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT8;
            }
            else if(inDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT16;
            }
            else if(inDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT32;
            }
            else if(inDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT64;
            }
            else if(inDataType == kea_32float)
            {
                imgBandDT = H5::PredType::NATIVE_FLOAT;
            }
            else if(inDataType == kea_64float)
            {
                imgBandDT = H5::PredType::NATIVE_DOUBLE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            // OPEN BAND DATASET AND WRITE IMAGE DATA
            try 
            {
                std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
                H5::DataSpace imgBandDataspace = imgBandDataset.getSpace();                
                
                hsize_t imgOffset[2];
                imgOffset[0] = yPxlOff;
                imgOffset[1] = xPxlOff;
                hsize_t dataDims[2];
                dataDims[0] = ySizeBuf;
                dataDims[1] = xSizeBuf;
                H5::DataSpace write2BandDataspace = H5::DataSpace(2, dataDims);
                
                if((ySizeOut != ySizeBuf) | (xSizeOut != xSizeBuf))
                {                    
                    hsize_t dataSelectMemDims[2];
                    dataSelectMemDims[0] = ySizeOut;
                    dataSelectMemDims[1] = 1;

                    hsize_t dataOffDims[2];
                    dataOffDims[0] = 0;
                    dataOffDims[1] = 0;

                    hsize_t dataSelectStrideDims[2];
                    dataSelectStrideDims[0] = 1;
                    if(xSizeBuf == xSizeOut)
                    {
                        dataSelectStrideDims[1] = 1;
                    }
                    else
                    {
                        dataSelectStrideDims[1] = xSizeBuf - xSizeOut;
                    }

                    hsize_t dataSelectBlockSizeDims[2];
                    dataSelectBlockSizeDims[0] = 1;
                    dataSelectBlockSizeDims[1] = xSizeOut;
                    write2BandDataspace.selectHyperslab(H5S_SELECT_SET, dataSelectMemDims, dataOffDims, dataSelectStrideDims, dataSelectBlockSizeDims);
                    
                    hsize_t dataOutDims[2];
                    dataOutDims[0] = ySizeOut;
                    dataOutDims[1] = xSizeOut;
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataOutDims, imgOffset);
                }
                else
                {
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataDims, imgOffset);
                }
                
                imgBandDataset.write( data, imgBandDT, write2BandDataspace, imgBandDataspace);
                
                imgBandDataset.close();
                imgBandDataspace.close();
                write2BandDataspace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not write image data.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
    }
    
    void KEAImageIO::readFromOverview(unsigned int band, unsigned int overview, void *data, unsigned long xPxlOff, unsigned long yPxlOff, unsigned long xSizeIn, unsigned long ySizeIn, unsigned long xSizeBuf, unsigned long ySizeBuf, KEADataType inDataType) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
            
            // GET NATIVE DATASET
            H5::DataType imgBandDT = H5::PredType::NATIVE_FLOAT;
            if(inDataType == kea_8int)
            {
                imgBandDT = H5::PredType::NATIVE_INT8;
            }
            else if(inDataType == kea_16int)
            {
                imgBandDT = H5::PredType::NATIVE_INT16;
            }
            else if(inDataType == kea_32int)
            {
                imgBandDT = H5::PredType::NATIVE_INT32;
            }
            else if(inDataType == kea_64int)
            {
                imgBandDT = H5::PredType::NATIVE_INT64;
            }
            else if(inDataType == kea_8uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT8;
            }
            else if(inDataType == kea_16uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT16;
            }
            else if(inDataType == kea_32uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT32;
            }
            else if(inDataType == kea_64uint)
            {
                imgBandDT = H5::PredType::NATIVE_UINT64;
            }
            else if(inDataType == kea_32float)
            {
                imgBandDT = H5::PredType::NATIVE_FLOAT;
            }
            else if(inDataType == kea_64float)
            {
                imgBandDT = H5::PredType::NATIVE_DOUBLE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            // OPEN BAND DATASET AND READ IMAGE DATA
            try 
            {
                std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
                H5::DataSpace imgBandDataspace = imgBandDataset.getSpace();                
                
                hsize_t dataOffset[2];
                dataOffset[0] = yPxlOff;
                dataOffset[1] = xPxlOff;
                hsize_t dataDims[2];
                dataDims[0] = ySizeBuf;
                dataDims[1] = xSizeBuf;
                H5::DataSpace read2BandDataspace = H5::DataSpace(2, dataDims);
                
                if((ySizeBuf != ySizeIn) | (xSizeBuf != xSizeIn))
                {
                    hsize_t dataSelectMemDims[2];
                    dataSelectMemDims[0] = ySizeIn;
                    dataSelectMemDims[1] = 1;

                    hsize_t dataOffDims[2];
                    dataOffDims[0] = 0;
                    dataOffDims[1] = 0;

                    hsize_t dataSelectStrideDims[2];
                    dataSelectStrideDims[0] = 1;
                    if(xSizeBuf == xSizeIn)
                    {
                        dataSelectStrideDims[1] = 1;
                    }
                    else
                    {
                        dataSelectStrideDims[1] = xSizeBuf - xSizeIn;
                    }

                    hsize_t dataSelectBlockSizeDims[2];
                    dataSelectBlockSizeDims[0] = 1;
                    dataSelectBlockSizeDims[1] = xSizeIn;
                    read2BandDataspace.selectHyperslab(H5S_SELECT_SET, dataSelectMemDims, dataOffDims, dataSelectStrideDims, dataSelectBlockSizeDims);
                    
                    hsize_t dataInDims[2];
                    dataInDims[0] = ySizeIn;
                    dataInDims[1] = xSizeIn;
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataInDims, dataOffset);
                }
                else
                {
                    imgBandDataspace.selectHyperslab( H5S_SELECT_SET, dataDims, dataOffset);
                }
                imgBandDataset.read( data, imgBandDT, read2BandDataspace, imgBandDataspace);
                
                imgBandDataset.close();
                imgBandDataspace.close();
                read2BandDataspace.close();
            } 
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not read from image overview.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
    }
    
    unsigned int KEAImageIO::getNumOfOverviews(unsigned int band) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        std::string overviewGroupName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_BANDNAME_OVERVIEWS;
        unsigned int numOverviews = 0;
        try 
        {
            // Try to open dataset with overviewName
            H5::Group imgOverviewsGrp = this->keaImgFile->openGroup(overviewGroupName);
            numOverviews = imgOverviewsGrp.getNumObjs();
        }
        catch (H5::Exception &e)
        {
            throw KEAIOException("Could not retrieve the number of image band overviews.");
        }
        
        return numOverviews;
    }
    
    void KEAImageIO::getOverviewSize(unsigned int band, unsigned int overview, unsigned long *xSize, unsigned long *ySize) throw(KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            // CHECK PARAMETERS PROVIDED FIT WITHIN IMAGE
            if(band == 0)
            {
                throw KEAIOException("KEA Image Bands start at 1.");
            }
            else if(band > this->numImgBands)
            {
                throw KEAIOException("Band is not present within image."); 
            }
                        
            // OPEN BAND DATASET AND READ THE IMAGE DIMENSIONS
            try 
            {
                std::string overviewName = KEA_DATASETNAME_BAND + uint2Str(band) + KEA_OVERVIEWSNAME_OVERVIEW + uint2Str(overview);
                H5::DataSet imgBandDataset = this->keaImgFile->openDataSet( overviewName );
                H5::DataSpace imgBandDataspace = imgBandDataset.getSpace();                
                
                unsigned int nDims = imgBandDataspace.getSimpleExtentNdims();
                if(nDims != 2)
                {
                    throw KEAIOException("The number of dimensions for the overview must be 2.");
                }
                hsize_t *dims = new hsize_t[2];
                imgBandDataspace.getSimpleExtentDims(dims);
                
                *xSize = dims[1];
                *ySize = dims[0];
                
                delete[] dims;
                imgBandDataset.close();
            } 
            catch(KEAIOException &e)
            {
                throw e;
            }
            catch ( H5::Exception &e) 
            {
                throw KEAIOException("Could not read from image overview.");
            }            
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch( H5::Exception &e )
		{
			throw KEAIOException("Could not get the overview size.");
		}
    }
    
    KEAAttributeTable* KEAImageIO::getAttributeTable(KEAATTType type, unsigned int band) throw(KEAATTException, KEAIOException)
    {
        KEAAttributeTable *att = NULL;
        try 
        {
            if(type == kea_att_mem)
            {
                att = libkea::KEAAttributeTableInMem::createKeaAtt(this->keaImgFile, band);
            }
            else if(type == kea_att_cached)
            {
                throw KEAATTException("The cached implementation of the attribute table class has not yet been implemented.");
            }
            else
            {
                throw KEAATTException("The attribute table type was not recognised.");
            }
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch(KEAATTException &e)
        {
            throw e;
        }
        
        return att;
    }
    
    void KEAImageIO::setAttributeTable(KEAAttributeTable* att, unsigned int band, unsigned int chunkSize, unsigned int deflate) throw(KEAATTException, KEAIOException)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        try 
        {
            att->exportToKeaFile(this->keaImgFile, band, chunkSize, deflate);
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch(KEAATTException &e)
        {
            throw e;
        }
    }
    
    bool KEAImageIO::attributeTablePresent(unsigned int band)
    {
        if(!this->fileOpen)
        {
            throw KEAIOException("Image was not open.");
        }
        
        bool attPresent = false;
        try 
        {
            std::string bandPathBase = KEA_DATASETNAME_BAND + uint2Str(band);
            hsize_t *attSize = new hsize_t[5];
            try 
            {   
                hsize_t dimsValue[1];
                dimsValue[0] = 5;
                H5::DataSpace valueDataSpace(1, dimsValue);
                H5::DataSet datasetAttSize = this->keaImgFile->openDataSet( bandPathBase + KEA_ATT_SIZE_HEADER );
                datasetAttSize.read(attSize, H5::PredType::STD_U64LE, valueDataSpace);
                datasetAttSize.close();
                valueDataSpace.close();
            } 
            catch (H5::Exception &e) 
            {
                throw KEAIOException("The attribute table size field is not present.");
            }
            
            if(attSize[0] > 0)
            {
                attPresent = true;
            }
            
            delete[] attSize;
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
        catch(KEAATTException &e)
        {
            throw e;
        }
        catch( H5::Exception &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        return attPresent;
    }
    
    void KEAImageIO::close() throw(KEAIOException)
    {
        try 
        {
            delete this->spatialInfoFile;
            this->keaImgFile->flush(H5F_SCOPE_GLOBAL);
            this->keaImgFile = NULL;
            this->fileOpen = false;
        }
        catch(KEAIOException &e)
        {
            throw e;
        }
    }
        
    H5::H5File* KEAImageIO::createKEAImage(std::string fileName, KEADataType dataType, unsigned int xSize, unsigned int ySize, unsigned int numImgBands, std::vector<std::string> *bandDescrips, KEAImageSpatialInfo * spatialInfo, unsigned int imageBlockSize, unsigned int attBlockSize, int mdcElmts, hsize_t rdccNElmts, hsize_t rdccNBytes, double rdccW0, hsize_t sieveBuf, hsize_t metaBlockSize, unsigned int deflate)throw(KEAIOException)
    {
        H5::Exception::dontPrint();
        
        H5::H5File *keaImgH5File = NULL;
        
        try 
        {
            // CREATE HDF FILE ACCESS PROPERTIES - DEFAULT VALUES CAN BE TUNED FROM KEACommon.h
            H5::FileAccPropList keaAccessPlist = H5::FileAccPropList(H5::FileAccPropList::DEFAULT);
            keaAccessPlist.setCache(mdcElmts, rdccNElmts, rdccNBytes, rdccW0);
            keaAccessPlist.setSieveBufSize(sieveBuf);
            keaAccessPlist.setMetaBlockSize(metaBlockSize);
            
            // CREATE THE HDF FILE - EXISTING FILE WILL BE TRUNCATED
            keaImgH5File = new H5::H5File( fileName, H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, keaAccessPlist);
            
            //////////// CREATE GLOBAL HEADER ////////////////
            keaImgH5File->createGroup( KEA_DATASETNAME_HEADER );
            
            bool deleteSpatialInfo = false;
            if(spatialInfo == NULL)
            {
                spatialInfo = new KEAImageSpatialInfo();
                spatialInfo->tlX = 0.0;
                spatialInfo->tlY = 0.0;
                spatialInfo->xRes = 1.0;
                spatialInfo->yRes = -1.0;
                spatialInfo->xRot = 0.0;
                spatialInfo->yRot = 0.0;
                spatialInfo->wktString = "";
                deleteSpatialInfo = true;
            }
            
            spatialInfo->xSize = xSize;
            spatialInfo->ySize = ySize;
                        
            // SET NUMBER OF IMAGE BANDS IN GLOBAL HEADER
            hsize_t dimsNumBands[1];
			dimsNumBands[0] = 1;
            H5::DataSpace numBandsDataSpace(1, dimsNumBands);
            H5::DataSet numBandsDataset = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_NUMBANDS, H5::PredType::STD_U16LE, numBandsDataSpace);
			numBandsDataset.write( &numImgBands, H5::PredType::NATIVE_UINT );
            numBandsDataset.close();
            numBandsDataSpace.close();
                                    
            // SET X AND Y TL IN GLOBAL HEADER
            double *doubleVals = new double[2];
            doubleVals[0] = spatialInfo->tlX;
            doubleVals[1] = spatialInfo->tlY;
            hsize_t dimsSpatialTL[1];
			dimsSpatialTL[0] = 2;
            H5::DataSpace spatialTLDataSpace(1, dimsSpatialTL);
            H5::DataSet spatialTLDataset = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_TL, H5::PredType::IEEE_F64LE, spatialTLDataSpace);
			spatialTLDataset.write( doubleVals, H5::PredType::NATIVE_DOUBLE );
            spatialTLDataset.close();
            spatialTLDataSpace.close();
            delete[] doubleVals;
            
            // SET X AND Y RESOLUTION IN GLOBAL HEADER
            float *floatVals = new float[2];
            floatVals[0] = spatialInfo->xRes;
            floatVals[1] = spatialInfo->yRes;
            hsize_t dimsSpatialRes[1];
			dimsSpatialRes[0] = 2;
            H5::DataSpace spatialResDataSpace(1, dimsSpatialRes);
            H5::DataSet spatialResDataset = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_RES, H5::PredType::IEEE_F32LE, spatialResDataSpace);
			spatialResDataset.write( floatVals, H5::PredType::NATIVE_FLOAT );
            spatialResDataset.close();
            spatialResDataSpace.close();
            
            // SET X AND Y ROTATION IN GLOBAL HEADER
            floatVals[0] = spatialInfo->xRot;
            floatVals[1] = spatialInfo->yRot;
            hsize_t dimsSpatialRot[1];
			dimsSpatialRot[0] = 2;
            H5::DataSpace spatialRotDataSpace(1, dimsSpatialRot);
            H5::DataSet spatialRotDataset = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_ROT, H5::PredType::IEEE_F32LE, spatialRotDataSpace);
			spatialRotDataset.write( floatVals, H5::PredType::NATIVE_FLOAT );
            spatialRotDataset.close();
            spatialRotDataSpace.close();
            delete[] floatVals;
                        
            // SET NUMBER OF X AND Y PIXELS
            unsigned long *uLongVals = new unsigned long[2];
            uLongVals[0] = spatialInfo->xSize;
            uLongVals[1] = spatialInfo->ySize;
            hsize_t dimsSpatialSize[1];
			dimsSpatialSize[0] = 2;
            H5::DataSpace spatialSizeDataSpace(1, dimsSpatialSize);
            H5::DataSet spatialSizeDataset = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_SIZE, H5::PredType::STD_U64LE, spatialSizeDataSpace);
			spatialSizeDataset.write( uLongVals, H5::PredType::NATIVE_ULONG );
            spatialSizeDataset.close();
            spatialSizeDataSpace.close();
            delete[] uLongVals;
            
            // SET THE WKT STRING SPATAIL REFERENCE IN GLOBAL HEADER
			const char **wStrdata = NULL;
			hsize_t	dimsForStr[1];
			dimsForStr[0] = 1; // number of lines;
            H5::DataSpace dataspaceStrAll(1, dimsForStr);
            H5::StrType strTypeAll(0, H5T_VARIABLE);
            H5::DataSet datasetSpatialReference = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_WKT, strTypeAll, dataspaceStrAll);
			wStrdata = new const char*[1];
			wStrdata[0] = spatialInfo->wktString.c_str();			
			datasetSpatialReference.write((void*)wStrdata, strTypeAll);
			datasetSpatialReference.close();
			delete[] wStrdata;
            
            // SET THE FILE TYPE IN GLOBAL HEADER
            H5::DataSet datasetFileType = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_FILETYPE, strTypeAll, dataspaceStrAll);
			wStrdata = new const char*[1];
            std::string strVal = "KEA";
			wStrdata[0] = strVal.c_str();			
			datasetFileType.write((void*)wStrdata, strTypeAll);
			datasetFileType.close();
			delete[] wStrdata;
            
            // SET THE FILE GENARATOR IN GLOBAL HEADER
            H5::DataSet datasetGenarator = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_GENERATOR, strTypeAll, dataspaceStrAll);
			wStrdata = new const char*[1];
            strVal = "LibKEA";
			wStrdata[0] = strVal.c_str();			
			datasetGenarator.write((void*)wStrdata, strTypeAll);
			datasetGenarator.close();
			delete[] wStrdata;
            
            // SET THE FILE VERSION IN GLOBAL HEADER
            H5::DataSet datasetVersion = keaImgH5File->createDataSet(KEA_DATASETNAME_HEADER_VERSION, strTypeAll, dataspaceStrAll);
			wStrdata = new const char*[1];
            strVal = "1.0";
			wStrdata[0] = strVal.c_str();			
			datasetVersion.write((void*)wStrdata, strTypeAll);
			datasetVersion.close();
			delete[] wStrdata;
            
            if(deleteSpatialInfo)
            {
                delete spatialInfo;
            }
            //////////// CREATED GLOBAL HEADER ////////////////
            
            //////////// CREATE GLOBAL META-DATA ////////////////
			keaImgH5File->createGroup( KEA_DATASETNAME_METADATA );
            //////////// CREATED GLOBAL META-DATA ////////////////
            
            
            //////////// CREATE IMAGE BANDS ////////////////
            bool bandDescriptsDefined = true;
            if(bandDescrips == NULL)
            {
                bandDescriptsDefined = false;
            }
            
            H5::DataType imgBandDT = H5::PredType::IEEE_F32LE;
            if(dataType == kea_8int)
            {
                imgBandDT = H5::PredType::STD_I8LE;
            }
            else if(dataType == kea_16int)
            {
                imgBandDT = H5::PredType::STD_I16LE;
            }
            else if(dataType == kea_32int)
            {
                imgBandDT = H5::PredType::STD_I32LE;
            }
            else if(dataType == kea_64int)
            {
                imgBandDT = H5::PredType::STD_I64LE;
            }
            else if(dataType == kea_8uint)
            {
                imgBandDT = H5::PredType::STD_U8LE;
            }
            else if(dataType == kea_16uint)
            {
                imgBandDT = H5::PredType::STD_U16LE;
            }
            else if(dataType == kea_32uint)
            {
                imgBandDT = H5::PredType::STD_U32LE;
            }
            else if(dataType == kea_64uint)
            {
                imgBandDT = H5::PredType::STD_U64LE;
            }
            else if(dataType == kea_32float)
            {
                imgBandDT = H5::PredType::IEEE_F32LE;
            }
            else if(dataType == kea_64float)
            {
                imgBandDT = H5::PredType::IEEE_F64LE;
            }
            else
            {
                throw KEAIOException("The specified data type was not recognised.");
            }
            
            int initFillVal = 0;
            
            hsize_t imageBandDims[2];
            imageBandDims[0] = ySize;
            imageBandDims[1] = xSize;
            H5::DataSpace imgBandDataSpace(2, imageBandDims);
            
            // Find the smallest axis of the image.
            unsigned long minImgDim = ySize; 
            if(xSize < ySize)
            {
                minImgDim = xSize;
            }
            
            unsigned int blockSize2Use = imageBlockSize;
            if(imageBlockSize > minImgDim)
            {
                blockSize2Use = minImgDim;
            }
            
            hsize_t dimsImageBandChunk[2];
			dimsImageBandChunk[0] = blockSize2Use;
			dimsImageBandChunk[1] = blockSize2Use;
            
            H5::DSetCreatPropList initParamsImgBand;
			initParamsImgBand.setChunk(2, dimsImageBandChunk);			
			initParamsImgBand.setShuffle();
            initParamsImgBand.setDeflate(deflate);
			initParamsImgBand.setFillValue( H5::PredType::NATIVE_INT, &initFillVal);
            
            H5::StrType strdatatypeLen6(H5::PredType::C_S1, 6);
            H5::StrType strdatatypeLen4(H5::PredType::C_S1, 4);
            const H5std_string strClassVal ("IMAGE");
            const H5std_string strImgVerVal ("1.2");
            H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
            
            unsigned int bandType = kea_continuous;
            std::string bandName = "";
            std::string bandDescrip = "";
            for(unsigned int i = 0; i < numImgBands; ++i)
            {
                bandName = KEA_DATASETNAME_BAND + uint2Str(i+1);
                
                // CREATE IMAGE BAND HDF5 GROUP
                keaImgH5File->createGroup( bandName );
                
                
                // CREATE THE IMAGE DATA ARRAY
                H5::DataSet imgBandDataSet = keaImgH5File->createDataSet((bandName+KEA_BANDNAME_DATA), imgBandDT, imgBandDataSpace, initParamsImgBand);
                H5::Attribute classAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_CLASS, strdatatypeLen6, attr_dataspace);
                classAttribute.write(strdatatypeLen6, strClassVal); 
                classAttribute.close();
                
                H5::Attribute imgVerAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_IMAGE_VERSION, strdatatypeLen4, attr_dataspace);
                imgVerAttribute.write(strdatatypeLen4, strImgVerVal);
                imgVerAttribute.close();
                
                H5::Attribute blockSizeAttribute = imgBandDataSet.createAttribute(KEA_ATTRIBUTENAME_BLOCK_SIZE, H5::PredType::STD_U16LE, attr_dataspace);
                blockSizeAttribute.write(H5::PredType::NATIVE_UINT, &blockSize2Use);
                blockSizeAttribute.close();
                imgBandDataSet.close();
                
                // SET BAND NAME / DESCRIPTION
                if(bandDescriptsDefined && (i < bandDescrips->size()))
                {
                    bandDescrip = bandDescrips->at(i);
                }
                else
                {
                    bandDescriptsDefined = false;
                }
                
                if(!bandDescriptsDefined)
                {
                    bandDescrip = "Band " + uint2Str(i+1);
                }
                
                H5::DataSet datasetBandDescription = keaImgH5File->createDataSet((bandName+KEA_BANDNAME_DESCRIP), strTypeAll, dataspaceStrAll);
                wStrdata = new const char*[1];
                wStrdata[0] = bandDescrip.c_str();			
                datasetBandDescription.write((void*)wStrdata, strTypeAll);
                datasetBandDescription.close();
                delete[] wStrdata;
                
                // SET IMAGE BAND DATA TYPE IN IMAGE BAND
                hsize_t dimsDT[1];
                dimsDT[0] = 1;
                H5::DataSpace dtDataSpace(1, dimsDT);
                H5::DataSet dtDataset = keaImgH5File->createDataSet((bandName+KEA_BANDNAME_DT), H5::PredType::STD_U16LE, dtDataSpace);
                dtDataset.write( &dataType, H5::PredType::NATIVE_UINT );
                dtDataset.close();
                
                // SET IMAGE BAND TYPE IN IMAGE BAND (I.E., CONTINUOUS (0) OR THEMATIC (1))
                hsize_t dimsType[1];
                dimsType[0] = 1;
                H5::DataSpace typeDataSpace(1, dimsType);
                H5::DataSet typeDataset = keaImgH5File->createDataSet((bandName+KEA_BANDNAME_TYPE), H5::PredType::STD_U8LE, typeDataSpace);
                typeDataset.write( &bandType, H5::PredType::NATIVE_UINT );
                typeDataset.close();
                
                // CREATE META-DATA
                keaImgH5File->createGroup( bandName+KEA_BANDNAME_METADATA );
                
                // CREATE OVERVIEWS GROUP
                keaImgH5File->createGroup( bandName+KEA_BANDNAME_OVERVIEWS );
                
                // CREATE ATTRIBUTE TABLE GROUP
                keaImgH5File->createGroup( bandName+KEA_BANDNAME_ATT );
                keaImgH5File->createGroup( bandName+KEA_ATT_GROUPNAME_DATA );
                keaImgH5File->createGroup( bandName+KEA_ATT_GROUPNAME_NEIGHBOURS );
                keaImgH5File->createGroup( bandName+KEA_ATT_GROUPNAME_HEADER );
                
                // SET ATTRIBUTE TABLE CHUNK SIZE
                int attChunkSize = 0;
                hsize_t dimsAttChunkSize[1];
                dimsAttChunkSize[0] = 1;
                H5::DataSpace attChunkSizeDataSpace(1, dimsAttChunkSize);
                H5::DataSet attChunkSizeDataset = keaImgH5File->createDataSet((bandName+KEA_ATT_CHUNKSIZE_HEADER), H5::PredType::STD_U64LE, attChunkSizeDataSpace);
                attChunkSizeDataset.write( &attChunkSize, H5::PredType::NATIVE_INT );
                attChunkSizeDataset.close();
                attChunkSizeDataSpace.close();
                
                // SET ATTRIBUTE TABLE SIZE
                int *attSize = new int[5];
                attSize[0] = 0;
                attSize[1] = 0;
                attSize[2] = 0;
                attSize[3] = 0;
                attSize[4] = 0;
                hsize_t dimsAttSize[1];
                dimsAttSize[0] = 5;
                H5::DataSpace attSizeDataSpace(1, dimsAttSize);
                H5::DataSet attSizeDataset = keaImgH5File->createDataSet((bandName+KEA_ATT_SIZE_HEADER), H5::PredType::STD_U64LE, attSizeDataSpace);
                attSizeDataset.write( attSize, H5::PredType::NATIVE_INT );
                attSizeDataset.close();
                attSizeDataSpace.close();
            }
            dataspaceStrAll.close();
            attr_dataspace.close();
            //////////// CREATED IMAGE BANDS ////////////////
        
            
        }
        catch (KEAIOException &e) 
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        
        return keaImgH5File;
    }
    
    H5::H5File* KEAImageIO::openKeaH5RW(std::string fileName, int mdcElmts, hsize_t rdccNElmts, hsize_t rdccNBytes, double rdccW0, hsize_t sieveBuf, hsize_t metaBlockSize)throw(KEAIOException)
    {
        H5::Exception::dontPrint();
        
        H5::H5File *keaImgH5File = NULL;
        try 
        {
            H5::FileAccPropList keaAccessPlist = H5::FileAccPropList(H5::FileAccPropList::DEFAULT);
            keaAccessPlist.setCache(mdcElmts, rdccNElmts, rdccNBytes, rdccW0);
            keaAccessPlist.setSieveBufSize(sieveBuf);
            keaAccessPlist.setMetaBlockSize(metaBlockSize);
            
            const H5std_string keaImgFilePath(fileName);
            keaImgH5File = new H5::H5File(keaImgFilePath, H5F_ACC_RDWR, H5::FileCreatPropList::DEFAULT, keaAccessPlist);
        } 
        catch (KEAIOException &e) 
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        
        return keaImgH5File;
    }
    
    H5::H5File* KEAImageIO::openKeaH5RDOnly(std::string fileName, int mdcElmts, hsize_t rdccNElmts, hsize_t rdccNBytes, double rdccW0, hsize_t sieveBuf, hsize_t metaBlockSize)throw(KEAIOException)
    {
        H5::Exception::dontPrint();
        
        H5::H5File *keaImgH5File = NULL;
        try 
        {
            H5::FileAccPropList keaAccessPlist = H5::FileAccPropList(H5::FileAccPropList::DEFAULT);
            keaAccessPlist.setCache(mdcElmts, rdccNElmts, rdccNBytes, rdccW0);
            keaAccessPlist.setSieveBufSize(sieveBuf);
            keaAccessPlist.setMetaBlockSize(metaBlockSize);
            
            const H5std_string keaImgFilePath(fileName);
            keaImgH5File = new H5::H5File(keaImgFilePath, H5F_ACC_RDONLY, H5::FileCreatPropList::DEFAULT, keaAccessPlist);
        } 
        catch (KEAIOException &e) 
        {
            throw e;
        }
        catch( H5::FileIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSetIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataSpaceIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
		catch( H5::DataTypeIException &e )
		{
			throw KEAIOException(e.getCDetailMsg());
		}
        
        return keaImgH5File;
    }
        
    bool KEAImageIO::isKEAImage(std::string fileName)throw(KEAIOException)
    {
        bool keaImageFound = false;
        H5::Exception::dontPrint();
        
        try 
        {
            H5::H5File *keaImgH5File = NULL;
            const H5std_string keaImgFilePath(fileName);
            keaImgH5File = new H5::H5File(keaImgFilePath, H5F_ACC_RDONLY);
            
            try 
            {
                hid_t nativeFTStrType;
                H5::DataSet datasetFileType = keaImgH5File->openDataSet( KEA_DATASETNAME_HEADER_FILETYPE );
                H5::DataType strFTDataType = datasetFileType.getDataType();
                char **strFTData = new char*[1];
                if((nativeFTStrType=H5Tget_native_type(strFTDataType.getId(), H5T_DIR_DEFAULT))<0)
                {
                    throw KEAIOException("Could not define a native string type");
                }
                datasetFileType.read((void*)strFTData, strFTDataType);
                std::string fileType = std::string(strFTData[0]);
                delete strFTData[0];
                delete[] strFTData;
                datasetFileType.close();
                                
                try 
                {
                    if(fileType == "KEA")
                    {
                        hid_t nativeVerStrType;
                        H5::DataSet datasetFileVersion = keaImgH5File->openDataSet( KEA_DATASETNAME_HEADER_VERSION );
                        H5::DataType strVerDataType = datasetFileVersion.getDataType();
                        char **strVerData = new char*[1];
                        if((nativeVerStrType=H5Tget_native_type(strVerDataType.getId(), H5T_DIR_DEFAULT))<0)
                        {
                            throw KEAIOException("Could not define a native string type");
                        }
                        datasetFileVersion.read((void*)strVerData, strVerDataType);
                        std::string fileVersion = std::string(strVerData[0]);
                        delete strVerData[0];
                        delete[] strVerData;
                        datasetFileVersion.close();
                        
                        if(fileVersion == "1.0")
                        {
                            keaImageFound = true;
                        }
                        else
                        {
                            keaImageFound = false;
                        }
                    }
                    else
                    {
                        keaImageFound = false;
                    }
                } 
                catch ( H5::Exception &e) // WILL BE THROWN IF THE FILE VERSION DATASET IS NOT PRESENT
                {
                    keaImageFound = false;
                }
                
            } 
            catch ( H5::Exception &e) // WILL BE THROWN IF THE FILE TYPE DATASET IS NOT PRESENT
            {
                keaImageFound = false;
            }
            
            keaImgH5File->close();
        } 
        catch( H5::Exception &e ) // WILL BE THROWN WHEN THE HDF LIBRARY CANNOT OPEN THE FILE - IE IT IS NOT A HDF5 FILE!
		{
			keaImageFound = false;
		}
        
        return keaImageFound;
    }

    KEAImageIO::~KEAImageIO()
    {
        
    }
}





