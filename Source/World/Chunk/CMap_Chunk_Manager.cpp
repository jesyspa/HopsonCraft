#include "CMap.h"

#include "../../Maths/General_Maths.h"
#include "../../Camera.h"

namespace
{
    int8_t m_renderDistance    = 15;
    int8_t m_currentLoadDist   = 1;

    struct Load_Sector
    {
        Load_Sector(int32_t minX, int32_t maxX,
                    int32_t minZ, int32_t maxZ)
        :   minX    (minX)
        ,   maxX    (maxX)
        ,   minZ    (minZ)
        ,   maxZ    (maxZ)
        { }

        int32_t minX, maxX,
                minZ, maxZ;
    };
}

namespace Chunk
{
    namespace
    {
        template<typename T, typename R>
        void loadDistChange(T& ld, R rd)
        {
            if (ld < rd - 1)
            {
                ld++;
            }
            if (ld - 1 > rd) ld = 1;
        }
    }

    void Map::manageChunks()
    {
        loadAndGenChunks();
        loadDistChange(m_currentLoadDist, m_renderDistance);

        flagChunks();
    }

    void Map::loadAndGenChunks()
    {
        //auto pos = Maths::worldToChunkPos(m_p_camera->position);
/*
        Load_Sector sect
        (
            pos.x - m_currentLoadDist,
            pos.x + m_currentLoadDist,
            pos.y - m_currentLoadDist,
            pos.y + m_currentLoadDist
        );
*/
        Load_Sector sect
        (
            0,
            m_currentLoadDist,
            0,
            m_currentLoadDist
        );

        for (auto x = sect.minX; x < sect.maxX; x++)
        {
            for (auto z = sect.minZ; z < sect.maxZ; z++)
            {
                addChunk({x, z});
            }
        }

        for (auto x = sect.minX; x < sect.maxX; x++)
        {
            for (auto z = sect.minZ; z < sect.maxZ; z++)
            {
                auto* chunk =  getChunk({x, z});
                if (chunk)
                {
                    if (!chunk->getFlags().hasFullMesh)
                    {
                        chunk->createFullMesh();
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    void Map::flagChunks()
    {
        auto pos = Maths::worldToChunkPos(m_p_camera->position);
        Load_Sector deleteSect
        (
            pos.x - m_renderDistance - 1,
            pos.x + m_renderDistance + 1,
            pos.y - m_renderDistance - 1,
            pos.y + m_renderDistance + 1
        );

        for (auto itr = m_chunks.begin(); itr != m_chunks.end(); itr++)
        {
            auto& c = *itr->second;
            Position p = c.getPosition();
            if (p.x < deleteSect.minX ||
                p.x > deleteSect.maxX ||
                p.y < deleteSect.minZ ||
                p.y > deleteSect.maxZ)
            {
                if(!c.getFlags().deleteMe)
                {
                    c.setDeleteFlag(true);
                    m_chunksToDelete.push_back(c.getPosition());
                }
            }
        }
    }
}
