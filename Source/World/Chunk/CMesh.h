#ifndef CMESH_H_INCLUDED
#define CMESH_H_INCLUDED

#include "../../Model.h"
#include "../Block/Block_Position.h"

class Chunklet;

namespace Block
{
    class Data;
};

namespace Texture
{
    class Atlas;
}

namespace Chunk
{
    class Mesh_Section
    {
        public:
            void reset();

            void buffer ();

            void addVerticies   (const std::vector<GLfloat>& v);
            void addTexCoords   (const std::vector<GLfloat>& t);
            void addIndices     (const std::vector<GLuint>&  i);
            void addLightVal    (GLfloat cardinalVal);

            const Model& getModel() const;

            void addIndices();

            uint32_t getFacesCount() const { return m_faces; }

        private:
            std::vector<GLfloat>    m_verticies;
            std::vector<GLfloat>    m_texCoords;
            std::vector<GLuint>     m_indices;
            std::vector<GLfloat>    m_light;

            Model m_model;

            uint32_t m_faces = 0;

            GLuint m_indicesCount = 0;
    };

    class Mesh
    {
        public:
            Mesh(Chunklet& chunklet);

            void create();
            void buffer();

            const Mesh_Section& getSolidMesh    () const;
            const Mesh_Section& getLiquidMesh   () const;


        private:
            void setActiveSection();

            bool shouldMakeFaceAdjacentTo(const Block::Small_Position& pos);

            void makeFrontFace  (const Block::Small_Position& pos);
            void makeBackFace   (const Block::Small_Position& pos);
            void makeLeftFace   (const Block::Small_Position& pos);
            void makeRightFace  (const Block::Small_Position& pos);
            void makeTopFace    (const Block::Small_Position& pos);
            void makeBottomFace (const Block::Small_Position& pos);

            Mesh_Section m_solidMesh;
            Mesh_Section m_liquidMesh;
            Mesh_Section* m_activeSection = nullptr;

            Chunklet*               m_p_chunklet        = nullptr;
            const Block::Data*      m_p_activeBlockData = nullptr;
            const Texture::Atlas*   m_p_textureAtlas    = nullptr;
    };
}

#endif // CMESH_H_INCLUDED
