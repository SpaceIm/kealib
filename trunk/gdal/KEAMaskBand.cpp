/*
 *  KEAMaskBand.cpp
 *
 *  Created by Pete Bunting on 01/08/2012.
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

#include "KEAMaskBand.h"

// constructor
KEAMaskBand::KEAMaskBand(KEADataset *pDataset, int nSrcBand, GDALAccess eAccess,
                libkea::KEAImageIO *pImageIO, int *pRefCount)
 : KEARasterBand( pDataset, nSrcBand, eAccess, pImageIO, pRefCount )
{

}

KEAMaskBand::~KEAMaskBand()
{

}

// overridden implementation - calls readImageBlock2BandMask instead
CPLErr KEAMaskBand::IReadBlock( int nBlockXOff, int nBlockYOff, void * pImage )
{
    try
    {
        // GDAL deals in blocks - if we are at the end of a row
        // we need to adjust the amount read so we don't go over the edge
        int xsize = this->nBlockXSize;
        int xtotalsize = this->nBlockXSize * (nBlockXOff + 1);
        if( xtotalsize > this->nRasterXSize )
        {
            xsize -= (xtotalsize - this->nRasterXSize);
        }
        int ysize = this->nBlockYSize;
        int ytotalsize = this->nBlockYSize * (nBlockYOff + 1);
        if( ytotalsize > this->nRasterYSize )
        {
            ysize -= (ytotalsize - this->nRasterYSize);
        }
        this->m_pImageIO->readImageBlock2BandMask( this->nBand,
                                            pImage, this->nBlockXSize * nBlockXOff,
                                            this->nBlockYSize * nBlockYOff,
                                            xsize, ysize, this->nBlockXSize, this->nBlockYSize, 
                                            this->m_eKEADataType );
        return CE_None;
    }
    catch (libkea::KEAIOException &e)
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                "Failed to read file: %s", e.what() );
        return CE_Failure;
    }
}

// overridden implementation - calls writeImageBlock2BandMask instead
CPLErr KEAMaskBand::IWriteBlock( int nBlockXOff, int nBlockYOff, void * pImage )
{
    try
    {
        // GDAL deals in blocks - if we are at the end of a row
        // we need to adjust the amount written so we don't go over the edge
        int xsize = this->nBlockXSize;
        int xtotalsize = this->nBlockXSize * (nBlockXOff + 1);
        if( xtotalsize > this->nRasterXSize )
        {
            xsize -= (xtotalsize - this->nRasterXSize);
        }
        int ysize = this->nBlockYSize;
        int ytotalsize = this->nBlockYSize * (nBlockYOff + 1);
        if( ytotalsize > this->nRasterYSize )
        {
            ysize -= (ytotalsize - this->nRasterYSize);
        }

        this->m_pImageIO-> writeImageBlock2BandMask( this->nBand, 
                                            pImage, this->nBlockXSize * nBlockXOff,
                                            this->nBlockYSize * nBlockYOff,
                                            xsize, ysize, this->nBlockXSize, this->nBlockYSize,
                                            this->m_eKEADataType );
        return CE_None;
    }
    catch (libkea::KEAIOException &e)
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                "Failed to write file: %s", e.what() );
        return CE_Failure;
    }
}

const GDALRasterAttributeTable *KEAMaskBand::GetDefaultRAT()
{
    // KEARasterBand implements this, but we don't want to
    return NULL;
}

CPLErr KEAMaskBand::SetDefaultRAT(const GDALRasterAttributeTable *poRAT)
{
    // KEARasterBand implements this, but we don't want to
    return CE_Failure;    
}
