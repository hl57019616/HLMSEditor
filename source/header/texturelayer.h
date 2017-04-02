/****************************************************************************
**
** Copyright (C) 2016
**
** This file is generated by the Magus toolkit
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#ifndef TEXTURE_LAYER_H
#define TEXTURE_LAYER_H

#include <QThread>
#include "OgreRoot.h"
#include "OgreImage.h"
#include "OgreTexture.h"
#include "OgreMath.h"
#include "OgreVector2.h"
#include "OgreColourValue.h"
#include "OgreTexture.h"
#include "OgrePixelBox.h"
#include "OgreItem.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsDatablock.h"
#include "OgreHardwarePixelBuffer.h"

/****************************************************************************
 The TextureSaveWorker class is used to save a texture generation
 (used for undo/redo and set layer visible/invisible) in the background.
 ***************************************************************************/
class TextureSaveWorker : public QObject
{
    Q_OBJECT

    public:
        TextureSaveWorker::TextureSaveWorker (const Ogre::Image& image, const Ogre::String& filename) :
            mImage(image)
        {
            mImage = image;
            mFilename = filename;
        }

    public slots:
        void saveImage (void)
        {
            mImage.save(mFilename);
        }

    private:
        Ogre::Image mImage;
        Ogre::String mFilename;
 };

/****************************************************************************
 This class contains functions used by the Hlms editor painter window. A
 TextureLayer provides a reference to the texture atributes needed by the
 PaintLayer.
 A TextureLayer can be used by multiple PaintLayer objects.
 ***************************************************************************/
class TextureLayer : public QObject
{
    Q_OBJECT

	public:
        TextureLayer(void);
        ~TextureLayer(void);

        /* Set the id of the (current) datablock. This is the datablock internally used for painting.
         * In addition, the textureType must be set.
         * textureFileName - full qualified filename of the texture, on which is painted
         */
        void setDatablockIdAndTexture (const Ogre::IdString& datablockId,
                                       Ogre::PbsTextureTypes textureType,
                                       const Ogre::String& textureFileName);

        /* Blit the current pixelbox
         */
        void blitTexture (void);

        /* Load a texture file
         */
        void loadTextureGeneration (const Ogre::String& filename);

        /* Load a texture file, based on the sequence
         */
        void loadTextureGeneration (Ogre::ushort sequence);

        /* Save the texture as a temporary file, with a sequence number
         */
        void saveTextureGeneration (void);

        /* Save the texture as file in the import directory. The basename of the original filename
         * is extended with a 'timestamp'
         * The function returns the basename of the saved file
         */
        const Ogre::String& saveTextureWithTimeStampToImportDir (void);

        /* The setFirstTextureGeneration searches for the first (original) image in a sequence and loads it.
         */
        void setFirstTextureGeneration (void);

        /* The setlastTextureGeneration searches for the last image (identified by mMaxSequence) in a sequence and loads it.
         */
        void setLastTextureGeneration (void);

        /* Checks if a filename is an Ogre resource. The filename argument is a fully qualified filename or a base filename.
         */
        bool isTextureFileNameDefinedAsResource (const Ogre::String& filename);

        /* Checks if a file exists. The filename argument is a fully qualified filename
         */
        bool textureFileExists (const Ogre::String& filename);

        /* Set the name of the Carbon Copy texture. If it is a new filename, a new texture image is loaded
         */
        //void setCarbonCopyTextureFileName (const Ogre::String&  textureFileName);
        //const Ogre::String& getCarbonCopyTextureFileName (void);

        /* Set/get the Carbon Copye texture scale
         */
        //void setCarbonCopyScale (float scale);
        //float getCarbonCopyScale (void);


        Ogre::IdString mDatablockId;
        Ogre::PbsTextureTypes mTextureType;
        Ogre::String mTextureFileName;                          // Full qualified name of the texture file
        Ogre::uint8 mNumMipMaps;                                // Number of mipmaps of texture (on the GPU)
        Ogre::Image mOriginalTexture;                           // The original texture image
        Ogre::PixelBox mPixelboxOriginalTexture;                // Pixelbox of the original texture
        Ogre::Image mTextureOnWhichIsPainted;                   // Contains a texture Image; this image is uploaded to the texture layer of the Hlms (on GPU), every time
                                                                // the applyPaintEffect() function is called. This basically updates the texture layer in the Pbs.
                                                                // Initially, textureOnWhichIsPainted is the same image from disk, also loaded in the Pbs.
                                                                // mTextureOnWhichIsPainted is changed while painting. Intermediate versions of mTextureOnWhichIsPainted
                                                                // are stored (example: texturename.png, texturename_01.png, texturename_02.png. texturename_03.png, ...)
                                                                // and can be used for 'undo'.
        Ogre::PixelBox mPixelboxTextureOnWhichIsPainted;        // Pixelbox of mTextureOnWhichIsPainted; for speed purposes, it is created when the texture is set
        Ogre::uint32 mTextureOnWhichIsPaintedWidth;             // Width of mTextureOnWhichIsPainted; must be the same as the width of mTexture
        Ogre::uint32 mTextureOnWhichIsPaintedHeight;            // Height of mTextureOnWhichIsPainted; must be the same as the height of mTexture
        bool mTextureOnWhichIsPaintedHasAlpha;                  // Painting effect depends on the fact whether the texture has alpha enabled
        bool mTextureTypeDefined;                               // If true, a valid texture type is assigned to this layer
        Ogre::ushort mMaxSequence;                              // Max sequence number of the temporary texture file, used for undo/redo functions

    protected:
        /* Returns the filename of a certain generation, defined by the sequence.
         * Assume the filename (mTextureFileName) is 'image.png'. The sequence is a value [0..mMaxSequence].
         * The following is returned:
         * sequence = 0                             ==> returned is 'image.png'
         * sequence = 1                             ==> returned is 'image1.png'
         * sequence = 2                             ==> returned is 'image2.png'
         * sequence = 3 (= mMaxSequence)            ==> returned is 'image3.png'
         * sequence = 4                             ==> returned is 'image3.png'
         * ... etc.
         * fullQualified indicates that either the basename is returned (fullQualified is false) or a full
         * qualified filename (fullQualified is true).
         */
        const Ogre::String& getTextureFileNameGeneration (int sequence, bool fullQualified = true);

        /* Create the texture used for the Carbon Copy effect
         */
        //void createCarbonCopyTexture (void);

    private:
        Ogre::String mHelperString;
};

#endif

