#include <Common.h>

#include <World/World.h>

#include <World/Queries/Range.h>

World::World()
	: m_entities(
			glm::vec3(-1000.0f, -1000.0f, -1000.0f), // Min
			glm::vec3(1000.0f, 1000.0f, 1000.0f), // Max
			glm::vec3(25.0f, 25.0f, 25.0f) // Cell Size
		)
{
}

World::~World()
{
	Delete();
}

void World::Delete()
{
	m_entities.clear();
	for (auto &pair : m_allEntities) {
		delete pair.second;
	}
	m_allEntities.clear();
}

void World::AddToOctree(Entity* ent)
{
	//TODO: Use cached WorldNode value to assert we're not already added to a node
	WorldNode &node = m_entities.getCell(ent->m_position);
	node.m_entities.push_back(ent);
}

void World::RemoveFromOctree(Entity* ent)
{
	//TODO: Cache the WorldNode inside of the entity so we can fetch it easily without having to traverse
	WorldNode &node = m_entities.getCell(ent->m_position);
	auto it = std::find(node.m_entities.begin(), node.m_entities.end(), ent);
	if (it == node.m_entities.end()) {
		assert(false);
		return;
	}
	node.m_entities.erase(it);
}

void World::RebuildOctree()
{
	//TODO: Clear cached WorldNode value before clearing and re-adding
	m_entities.clear();
	for (auto &pair : m_allEntities) {
		AddToOctree(pair.second);
	}
}

void World::QueryRange(const glm::vec3 &pos, float range, std::vector<Entity*> &out)
{
	WorldQueryRange query(pos, range);
	m_entities.traverse(query);
	for (Entity* ent : query.m_entities) {
		out.push_back(ent);
	}
}

void World::AddEntity(Entity* ent)
{
	m_allEntities[ent->m_handle] = ent;
	AddToOctree(ent);
}

void World::RemoveEntity(Entity* ent)
{
	RemoveFromOctree(ent);
	auto it = m_allEntities.find(ent->m_handle);
	if (it == m_allEntities.end()) {
		assert(false);
		return;
	}
	m_allEntities.erase(it);
}

void World::EntityMoved(Entity* ent, const glm::vec3 &oldPos)
{
	WorldNode &oldNode = m_entities.getCell(oldPos); //TODO: Use cached value instead!
	WorldNode &newNode = m_entities.getCell(ent->m_position);

	// We're still in the same node, no need to switch
	if (&oldNode == &newNode) {
		return;
	}

	auto it = std::find(oldNode.m_entities.begin(), oldNode.m_entities.end(), ent);
	if (it == oldNode.m_entities.end()) {
		assert(false);
	} else {
		oldNode.m_entities.erase(it);
	}

	newNode.m_entities.push_back(ent);
}

void World::Update()
{
	for (auto &pair : m_allEntities) {
		pair.second->Update();
	}
}