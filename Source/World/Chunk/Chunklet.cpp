#include "Chunklet.h"

#include "../../Maths/Matrix_Maths.h"
#include "../../File/World_File.h"
#include "CMap.h"

#include <iostream>

Chunklet::Chunklet(const Chunk::Chunklet_Position& pos,
                   Chunk::Map& map)
:   m_mesh          (*this)
,   m_pos           (pos)
,   m_p_chunkMap    (&map)
{
    position.x = pos.x * World_Constants::CH_SIZE;
    position.y = pos.y * World_Constants::CH_SIZE;
    position.z = pos.z * World_Constants::CH_SIZE;

    m_modelMat = Maths::createModelMatrix(*this);
}

void Chunklet::createMesh()
{
    m_flags.hasFaces = false;
    for(int x = -1; x <= 1; x++)
    {
        for(int z = -1; z <= 1; z++)
        {
            m_p_chunkMap->addChunk({m_pos.x + x, m_pos.z + z});
        }
    }
    m_mesh.create();
    m_flags.hasMesh = true;
    m_flags.hasBuffered = false;
}

void Chunklet::bufferMesh()
{
    m_mesh.buffer();
    m_flags.hasBuffered = true;
}


void Chunklet::qSetBlock(const Block::Small_Position& pos, CBlock block)
{
    m_blocks[getBlockIndex(pos)] = block;
}


CBlock Chunklet::getBlock(const Block::Small_Position& pos) const
{
    if (pos.x < 0)
    {
        auto x = m_pos.x - 1;
        auto z = m_pos.z;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);

        if (c)
        {
            return c->qGetBlock({World_Constants::CH_SIZE - 1, pos.y, pos.z});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else if (pos.x > World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x + 1;
        auto z = m_pos.z;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);

        if (c)
        {
            return c->qGetBlock({0, pos.y, pos.z});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else if (pos.z < 0)
    {
        auto x = m_pos.x;
        auto z = m_pos.z - 1;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);

        if (c)
        {
            return c->qGetBlock({pos.x, pos.y, World_Constants::CH_SIZE - 1});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else if (pos.z > World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x;
        auto z = m_pos.z + 1;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);

        if (c)
        {
            return c->qGetBlock({pos.x, pos.y, 0});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else if (pos.y < 0)
    {
        auto x = m_pos.x;
        auto z = m_pos.z;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y - 1);

        if (c)
        {
            return c->qGetBlock({pos.x, World_Constants::CH_SIZE - 1, pos.z});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else if (pos.y > World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x;
        auto z = m_pos.z;

        const Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y + 1);

        if (c)
        {
            return c->qGetBlock({pos.x, 0, pos.z});
        }
        else
        {
            return Block::ID::Air;
        }
    }
    else
    {
        return qGetBlock(pos);
    }
}

CBlock Chunklet::qGetBlock(const Block::Small_Position& pos) const
{
    return m_blocks[getBlockIndex(pos)];
}

std::vector<Chunklet*> Chunklet::setBlock(const Block::Small_Position& pos, CBlock block)
{
    std::vector<Chunklet*> chunks;
    chunks.push_back(this);
    if (pos.x == 0)
    {
        auto x = m_pos.x - 1;
        auto z = m_pos.z;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);
        if(c) chunks.push_back(c);
    }
    if (pos.x == World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x + 1;
        auto z = m_pos.z;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);
        if(c) chunks.push_back(c);
    }
    if (pos.z == 0)
    {
        auto x = m_pos.x;
        auto z = m_pos.z - 1;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);
        if(c) chunks.push_back(c);
    }
    if (pos.z == World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x;
        auto z = m_pos.z + 1;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y);
        if(c) chunks.push_back(c);
    }
    if (pos.y == 0)
    {
        auto x = m_pos.x;
        auto z = m_pos.z;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y - 1);
        if(c) chunks.push_back(c);
    }
    if (pos.y == World_Constants::CH_SIZE - 1)
    {
        auto x = m_pos.x;
        auto z = m_pos.z;

        Chunklet* c = m_p_chunkMap->getChunk({x, z})->getChunklet(m_pos.y + 1);
        if(c) chunks.push_back(c);
    }
    qSetBlock(pos, block);
    return chunks;
}


const Chunk::Mesh& Chunklet::getMesh() const
{
    return m_mesh;
}

const Matrix4& Chunklet::getMat() const
{
    return m_modelMat;
}

const Chunk::Chunklet_Position& Chunklet::getPosition() const
{
    return m_pos;
}

void Chunklet::setFaces(bool faces)
{
    m_flags.hasFaces = faces;
}

int32_t Chunklet::getBlockIndex(const Block::Small_Position& pos) const
{
    return pos.y * World_Constants::CH_AREA + pos.z * World_Constants::CH_SIZE + pos.x;
}

