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

        /* Set the name of the (current) datablock. This is the datablock internally used for painting.
         * In addition, the textureType must be set.
         * textureFileName
         * Set full qualified filename of the texture, on which is painted
         */
        void setDatablockNameAndTexture (const Ogre::IdString& datablockName,
                                         Ogre::PbsTextureTypes textureType,
                                         const Ogre::String& textureFileName);

        /* Blit the current pixelbox
         */
        void blitTexture (void);

        /* Save the texture as a temporary file, with a sequence number
         */
        void saveTextureGeneration (void);

        /* It is possible to move between generations of a texture layer. A generation is a set images with the same
         * base texture image. The images in the set represent a certain painting state.
         * This makes it possible to perform an undo or redo system.
         * The setNextTextureGeneration searches for the next image in a sequence and loads it.
         */
        //void setNextTextureGeneration (void);

        /* The setPreviousTextureGeneration searches for the previous image in a sequence and loads it.
         */
        //void setPreviousTextureGeneration (void);

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

        Ogre::IdString mDatablockName;
        Ogre::PbsTextureTypes mTextureType;
        Ogre::String mTextureFileName;                          // Full qualified name of the texture file
        Ogre::HlmsDatablock* mDatablock;                        // The datablock used to paint on
        Ogre::HlmsPbsDatablock* mDatablockPbs;                  // If the datablock is a Pbs, it is casted to a HlmsPbsDatablock*
        Ogre::TexturePtr mTexture;                              // The texture layer on which is painted
        Ogre::uint8 mNumMipMaps;                                // Number of mipmaps of mTexture (on the GPU)
        std::vector<Ogre::v1::HardwarePixelBuffer*> mBuffers;   // Texture buffers; depends on number of mipmaps
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

    private:
        Ogre::ushort mCurrentSequence;                          // Sequence number of the temporary texture file, used for undo/redo functions
        Ogre::ushort mMaxSequence;                              // Max sequence number of the temporary texture file, used for undo/redo functions
        Ogre::String mHelperString;
};

#endif

