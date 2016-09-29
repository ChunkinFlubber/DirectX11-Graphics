#include "pch.h"
#include "OBJLoader.h"


bool Object::loadOBJ(const char * path)
{
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<XMFLOAT3> temp_vertices, temp_normals;
	vector<XMFLOAT2> temp_uvs;
	FILE *file;
	fopen_s(&file, path, "r");
	if (file == NULL)
		return false;
	while (1)
	{
		char line[128];
		int res = fscanf_s(file, "%s", line, _countof(line));
		if (res == EOF)
			break;
		if (strcmp(line, "v") == 0)
		{
			XMFLOAT3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(line, "vt") == 0)
		{
			XMFLOAT2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(line, "vn") == 0)
		{
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(line, "f") == 0)
		{
			string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
			{
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
		DX11UWA::VERTEX vert;
		bool unique = true;
		unsigned int vertexIndex = vertexIndices[i];
		vert.pos = temp_vertices[vertexIndex - 1];
		unsigned int uvIndex = uvIndices[i];
		vert.uvs = temp_uvs[uvIndex - 1];
		unsigned int normalIndex = normalIndices[i];
		vert.normals = temp_normals[normalIndex - 1];
		for (unsigned int j = 0; j < verts.size(); ++j)
		{
			if (vert.pos.x == verts[j].pos.x && vert.pos.y == verts[j].pos.y && vert.pos.z == verts[j].pos.z && vert.uvs.x == verts[j].uvs.x && vert.uvs.y == verts[j].uvs.y && vert.normals.x == verts[j].normals.x && vert.normals.y == verts[j].normals.y && vert.normals.z == verts[j].normals.z)
			{
				unique = false;
				indexed.push_back(j);
				break;
			}
		}
		if (unique)
		{
			indexed.push_back(verts.size());
			verts.push_back(vert);
		}
	}

	return true;
}

bool Object::SetDX()
{

	return false;
}