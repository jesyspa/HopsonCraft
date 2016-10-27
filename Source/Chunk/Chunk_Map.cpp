#include "Chunk_Map.h"

#include <iostream>

#include "Maths/Position_Converter_Maths.h"
#include "Camera.h"
#include "Master_Renderer.h"

Chunk_Map::Chunk_Map(const Chunk_Location& playerPosition)
:   m_blockTextures     (1024, 32, "Blocks_Texture_Atlas")
,   m_playerPosition    (&playerPosition)
,   m_chunkManageThread (&Chunk_Map::manageChunks, this)
{
    m_chunkManageThread.detach();

    addChunk({0, 0});
    getChunkAt({0,0})->generateMesh();
    getChunkAt({0,0})->bufferMesh();
}

Chunk_Map::~Chunk_Map()
{
    m_isRunning = false;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

Chunk* Chunk_Map::getChunkAt (const Chunk_Location& location)
{
    if (m_chunks.find(location) != m_chunks.end())
    {
        return &*m_chunks.at(location);
    }
    else
    {
        return nullptr;
    }
}

void Chunk_Map::addChunk(const Chunk_Location& location)
{
    //m_accessMutex.lock();
    if (!getChunkAt(location))
    {
        m_chunks[location] = std::make_unique<Chunk>(location, *this, m_blockTextures);
    }
    //m_accessMutex.unlock();
}

void Chunk_Map::checkChunks()
{
    //m_accessMutex.lock();
    deleteChunks();
    //m_accessMutex.unlock();

    updateChunks();
}

void Chunk_Map::draw(Master_Renderer& renderer)
{
    //m_accessMutex.lock();
    m_blockTextures.bind();
    for (auto itr = m_chunks.begin() ; itr != m_chunks.end() ;)
    {
        Chunk* c = &*(itr)->second;  //So we don't have to dereference the iteraor which looks messy
        if (c->hasBuffered())
        {
            renderer.processChunk(*c);
            itr++;
        }
        else if (c->hasMesh())
        {
            c->bufferMesh();    //This is only for new chunks, when chunks are updated I just buffer the data there and then
        }
        else
        {
            itr++;
        }
    }
    //m_accessMutex.unlock();
}

void Chunk_Map::updateChunks()
{
    for ( auto itr = m_chunksToUpdate.begin() ; itr != m_chunksToUpdate.end() ; )
    {
        (*itr)->generateMesh();
        (*itr)->bufferMesh();
        itr = m_chunksToUpdate.erase( itr );
    }
}

void Chunk_Map::deleteChunks()
{
    for (auto itr = m_chunks.begin() ; itr != m_chunks.end() ;)
    {
        if (itr->second->hasDeleteFlag())
        {
            itr = m_chunks.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}

void Chunk_Map::setBlock (Block::Block_Base& block, const Vector3& worldPosition)
{
    auto addToBatch = [&](int x, int y)
    {
        if (getChunkAt({x, y}))
        {
            m_chunksToUpdate.push_back(getChunkAt({x, y}));
        }
    };

    Chunk_Location position (Maths::worldToChunkPosition(worldPosition));
    Vector3 blockPosition (Maths::worldToBlockPosition(worldPosition));

    auto* chunk = getChunkAt(position);
    if (chunk->getBlock(blockPosition).getID() == block.getID()) return;

    if (chunk)
    {
        chunk->setBlock(blockPosition, block);
        m_chunksToUpdate.push_back(chunk);
    }
    if (blockPosition.x == 0)
    {
        addToBatch(position.x - 1, position.z);
    }
    if (blockPosition.z == 0)
    {
        addToBatch(position.x, position.z - 1);
    }
    if (blockPosition.x == Chunk::SIZE)
    {
        addToBatch(position.x + 1, position.z);
    }
    if (blockPosition.z == Chunk::SIZE)
    {
        addToBatch(position.x, position.z + 1);
    }
}


struct Area
{
    Area(int minX, int minZ, int maxX, int maxZ)
    :   minX (minX)
    ,   minZ (minZ)
    ,   maxX (maxX)
    ,   maxZ (maxZ)
    { }

    int minX;
    int minZ;

    int maxX;
    int maxZ;
};


void Chunk_Map :: manageChunks()
{
    while (m_isRunning)
    {
        {
            Area loadArea
            (
                m_playerPosition->x - m_loadingDistance,
                m_playerPosition->z - m_loadingDistance,
                m_playerPosition->x + m_loadingDistance,
                m_playerPosition->z + m_loadingDistance
            );

            generateChunks( loadArea );
            if (!m_isRunning) return;
        }

        {
            Area deleteArea
            (
                m_playerPosition->x - m_renderDistance,
                m_playerPosition->z - m_renderDistance,
                m_playerPosition->x + m_renderDistance,
                m_playerPosition->z + m_renderDistance
            );


            flagChunksForDelete( deleteArea );
            if (!m_isRunning) return;
        }

        {
            Area generationArea
            (
                generationArea.minX =  m_playerPosition->x - m_generationDistance,
                generationArea.minZ =  m_playerPosition->z - m_generationDistance,
                generationArea.maxX =  m_playerPosition->x + m_generationDistance,
                generationArea.maxZ =  m_playerPosition->z + m_generationDistance
            );

            generateMeshes(generationArea);
            if (!m_isRunning) return; //Safety
        }


        if (m_loadingDistance < m_renderDistance + 2)
        {
            m_loadingDistance++;
        }
        else if (m_loadingDistance >= m_renderDistance + 2)
        {
            m_loadingDistance = 2;
        }

        if (m_generationDistance < m_renderDistance)
        {
            m_generationDistance++;
        }
        else if (m_generationDistance >= m_renderDistance)
        {
            m_generationDistance = 2;
        }
    }
}

void Chunk_Map::generateChunks (const Area& createArea)
{
    std::vector<std::unique_ptr<std::thread>> threads;

    for (int x = createArea.minX ; x < createArea.maxX ; x++)
    {
        if (!m_isRunning) return; //Safety
        for (int z = createArea.minZ ; z < createArea.maxZ ; z++)
        {
            if (!m_isRunning ) return; //Safety
            addChunk({x, z});
                /*
                threads.emplace_back(std::make_unique<std::thread>(&Chunk_Map::addChunk,
                                                                   this,
                                                                   Chunk_Location(x, z)));
                                                                   */
        }
        for (auto& thread : threads) thread->join();
        threads.clear();
    }
    for (auto& thread : threads) thread->join();
    threads.clear();
}

void Chunk_Map::flagChunksForDelete( const Area& deleteArea )
{
    for (auto& chunkPair : m_chunks)
    {
        if (!m_isRunning) return; //Safety

        Chunk& chunk = *chunkPair.second;
        if (chunk.hasDeleteFlag()) continue;

        Chunk_Location loc = chunk.getLocation();
        if (loc.x < deleteArea.minX ||
            loc.x > deleteArea.maxX ||
            loc.z < deleteArea.minZ ||
            loc.z > deleteArea.maxZ)
        {
            chunk.setToDelete();
        }
    }
}

void Chunk_Map::generateMeshes(const Area& generationArea)
{
    for (int x = generationArea.minX ; x < generationArea.maxX ; x++)
    {
        if (!m_isRunning) return; //Safety
        for (int z = generationArea.minZ ; z < generationArea.maxZ ; z++)
        {
            if (!m_isRunning) return; //Safety
            Chunk* chunk = getChunkAt({x, z});
            if (chunk)
            {
                if (!chunk->hasMesh() && chunk->hasBlockData())
                {
                    chunk->generateMesh();
                }
            }
        }
    }
}