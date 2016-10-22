#include "video.h"

cwsShader text_shader;
cwsMaterial text_material;

mat4 main_projection_matrix;
mat4 main_view_matrix;
cwsMaterial *active_material = NULL;

SDL_Window *main_window;
SDL_GLContext main_gl_context;

u32 gl_filters[] = {
	GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
};

void cwsVideoInit()
{
    cwsShaderInit(text_shader);
	cwsShaderFromfile(&text_shader, "./data/shaders/text_v", "./data/shaders/text_f", SH_NONE);

    cwsMaterialInit(text_material);
	text_material.shader = text_shader;
}

void cwsVideoDestroy()
{
    cwsDeleteShader(&text_shader);
}

//Allocates a new string and copies the data from src into the new returned buffer
//This returned buffer must be deleted!
char* copy_conststr(const char *src, i32 src_length)
{
	char *copy = (char*)malloc(sizeof(char) * src_length+1);
	for(i32 i = 0; i < src_length; ++i)
	{
		copy[i] = src[i];
	}

	copy[src_length] = '\0';
	return copy;
}

//Copies the node tree from Assimp to the internal format
void copy_node_tree(cwsNode *node, aiNode *anode)
{
	node->children = malloc(sizeof(cwsNode) * anode->mNumChildren);
	node->children_count = anode->mNumChildren;
	for(u32 i = 0; i < anode->mNumChildren; ++i)
	{
		node->children[i].anim_bone = NULL;
		node->children[i].current_anim_node = NULL;
		node->children[i].name = copy_conststr((const char*)anode->mChildren[i]->mName.data, anode->mChildren[i]->mName.length);

		aiMatrix4x4 ma = anode->mChildren[i]->mTransformation;
		node->children[i].transform = (mat4){.m[0] = ma.a1, .m[1] = ma.b1, .m[2] = ma.c1, .m[3] = ma.d1,
											 .m[4] = ma.a2, .m[5] = ma.b2, .m[6] = ma.c2, .m[7] = ma.d2,
											 .m[8] = ma.a3, .m[9] = ma.b3, .m[10] = ma.c3, .m[11] = ma.d3,
											 .m[12] = ma.a4, .m[13] = ma.b4, .m[14] = ma.c4, .m[15] = ma.d4};

		copy_node_tree(&node->children[i], anode->mChildren[i]);
	}
}

cwsNode *find_node_by_name(cwsNode *root, const char *name)
{
	if(strcmp(root->name,name) == 0)
	{
		return root;
	}

	for(i32 i = 0; i < root->children_count; ++i)
	{
		cwsNode *n = find_node_by_name(&root->children[i], name);
		if(n != NULL)
		{
			return n;
		}
	}

	return NULL;
}

void set_node_animnode(cwsNode *node, cwsAnimNode *an)
{
	if(strcmp(node->name,an->name) == 0)
	{
		node->current_anim_node = an;
		return;
	}

	for(i32 i = 0; i < node->children_count; ++i)
	{
		set_node_animnode(&node->children[i], an);
	}
}

bool cwsMeshFromfile(cwsMesh *mesh, const char *file)
{
	const aiScene *scene = aiImportFile(file, aiProcess_Triangulate 
												 | aiProcess_JoinIdenticalVertices 
												 | aiProcess_ImproveCacheLocality);

	if(scene == NULL)
	{
		cws_log("Couldn't load model %s[\n%s\n]", file, aiGetErrorString());
		return false;
	}

	i32 VERTEX_LENGTH = 0;
	f32 *vertex_data = NULL;
	i32 *index_data = NULL;
	i32 index_size = 0, index_offset = 0;
	i32 vertex_size = 0, vertex_offset = 0;

	//Prepare anim data
	if(scene->mNumAnimations > 0)
	{
		//The mesh ncwsds more vertex data if it supports animation
		i32 vertex_attribs[6] = {3, 3, 2, 3, 4, 4};
	    cwsEmptyMesh(mesh, vertex_attribs, 6);
		VERTEX_LENGTH = 19;

		aiMatrix4x4 m = scene->mRootNode->mTransformation;
		mesh->anim_data = (cwsMesh_AnimData*)malloc(sizeof(cwsMesh_AnimData));
		mesh->anim_data->animation_time = 0;
		mesh->anim_data->animations_count = 0;
		mesh->anim_data->animations = NULL;
		mat4 mm = (mat4){.m[0] = m.a1, .m[1] = m.b1, .m[2] = m.c1, .m[3] = m.d1,
					     .m[4] = m.a2, .m[5] = m.b2, .m[6] = m.c2, .m[7] = m.d2,
					     .m[8] = m.a3, .m[9] = m.b3, .m[10] = m.c3, .m[11] = m.d3,
					     .m[12] = m.a4, .m[13] = m.b4, .m[14] = m.c4, .m[15] = m.d4};
		mesh->anim_data->inverse_transform = mat4_inverse(mm);
		mesh->anim_data->root_node.name = copy_conststr((const char*)scene->mRootNode->mName.data, scene->mRootNode->mName.length);

		mesh->anim_data->root_node.transform = (mat4){.m[0] = m.a1, .m[1] = m.b1, .m[2] = m.c1, .m[3] = m.d1,
													  .m[4] = m.a2, .m[5] = m.b2, .m[6] = m.c2, .m[7] = m.d2,
													  .m[8] = m.a3, .m[9] = m.b3, .m[10] = m.c3, .m[11] = m.d3,
													  .m[12] = m.a4, .m[13] = m.b4, .m[14] = m.c4, .m[15] = m.d4};
		mesh->anim_data->root_node.anim_bone = NULL;
		mesh->anim_data->root_node.current_anim_node = NULL;
		mesh->anim_data->root_node.children = NULL;
		copy_node_tree(&mesh->anim_data->root_node, scene->mRootNode);
	}
	else
	{
		i32 vertex_attribs[4] = {3, 3, 2, 3};
		cwsEmptyMesh(mesh, vertex_attribs, 4);
		VERTEX_LENGTH = 11;
    }

	//Mesh bounds
	vec3 min,max;

	min = (vec3){.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX};
	max = (vec3){.x = FLT_MIN, .y = FLT_MIN, .z = FLT_MIN};

	/*
		Merge every single mesh from the file into a single EE_Mesh
	*/
	i32 vertex_index_offset = 0;
	i32 bone_index_offset = 0;
	for(u32 i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *m = scene->mMeshes[i];
		if(m == NULL)
		{
			cws_log("Error null mesh!");
            return false;
		}

		//Allocate space for mesh indices
		if(index_data == NULL)
		{
			index_data = (i32*) malloc(sizeof(i32) * m->mNumFaces*3);
			if(index_data == NULL)
			{
				cws_log("Error allocating memory for indices!");
				return false;
			}

			index_size = m->mNumFaces*3;
		}
		else
		{
			i32 *ind = (i32*)realloc(index_data, sizeof(i32) * (m->mNumFaces*3+index_size));
			if(ind == NULL)
			{
				cws_log("Error allocating memory for indices!");
				return false;
			}

			index_data = ind;
			index_size += m->mNumFaces*3;
		}

		//Get the indices
		vertex_index_offset = vertex_offset / VERTEX_LENGTH;
		for(u32 j = 0; j < m->mNumFaces; ++j)
		{
			index_data[index_offset] = m->mFaces[j].mIndices[0]+vertex_index_offset;
			index_data[index_offset+1] = m->mFaces[j].mIndices[1]+vertex_index_offset;
			index_data[index_offset+2] = m->mFaces[j].mIndices[2]+vertex_index_offset;

			index_offset += 3;
		}

		//Allocate space for mesh vertices
		if(vertex_data == NULL)
		{
			vertex_data = (f32*)malloc(sizeof(f32)*m->mNumVertices*VERTEX_LENGTH);

			if(vertex_data == NULL)
			{
				cws_log("Error allocating memory for vertex_data!");
				return false;
			}

			vertex_size = m->mNumVertices*VERTEX_LENGTH;
		}
		else
		{
			f32 *v = (f32*)realloc(vertex_data,sizeof(f32)* (vertex_size+(m->mNumVertices*VERTEX_LENGTH)));
			if(v == NULL)
			{
				cws_log("Error allocating memory for vertex_data!");
				return false;
			}

			vertex_data = v;
			vertex_size += m->mNumVertices*VERTEX_LENGTH;
		}

		for(u32 j = 0; j < m->mNumVertices; ++j)
		{
			//Get the mesh bounds
			if(m->mVertices[j].x < min.x)
				min.x = m->mVertices[j].x;
			if(m->mVertices[j].y < min.y)
				min.y = m->mVertices[j].y;
			if(m->mVertices[j].z < min.z)
				min.z = m->mVertices[j].z;

			if(m->mVertices[j].x > max.x)
				max.x = m->mVertices[j].x;
			if(m->mVertices[j].y > max.y)
				max.y = m->mVertices[j].y;
			if(m->mVertices[j].z > max.z)
				max.z = m->mVertices[j].z;

			vertex_data[vertex_offset] = m->mVertices[j].x;
			vertex_data[vertex_offset+1] = m->mVertices[j].y;
			vertex_data[vertex_offset+2] = m->mVertices[j].z;

			if(m->mNormals != NULL)
			{
				vertex_data[vertex_offset+3] = m->mNormals[j].x;
				vertex_data[vertex_offset+4] = m->mNormals[j].y;
				vertex_data[vertex_offset+5] = m->mNormals[j].z;
			}
			else
			{
				vertex_data[vertex_offset+3] = 0;
				vertex_data[vertex_offset+4] = 1;
				vertex_data[vertex_offset+5] = 0;	
			}

			if(m->mTextureCoords != NULL && m->mNumUVComponents[0] > 0)
			{
				vertex_data[vertex_offset+6] = m->mTextureCoords[0][j].x;
				vertex_data[vertex_offset+7] = 1.0f - m->mTextureCoords[0][j].y;
			}
			else
			{
				vertex_data[vertex_offset+6] = 0.0f;
				vertex_data[vertex_offset+7] = 0.0f;	
			}

			if(m->mColors[0] != NULL)
			{
				vertex_data[vertex_offset+8] = m->mColors[0][j].r;
				vertex_data[vertex_offset+9] = m->mColors[0][j].g;
				vertex_data[vertex_offset+10] = m->mColors[0][j].b;
			}
			else
			{
				vertex_data[vertex_offset+8] = 1.0f;
				vertex_data[vertex_offset+9] = 1.0f;
				vertex_data[vertex_offset+10] = 1.0f;	
			}

			if(scene->mNumAnimations > 0)
			{
				vertex_data[vertex_offset+11] = -1.0f;
				vertex_data[vertex_offset+12] = -1.0f;
				vertex_data[vertex_offset+13] = -1.0f;
				vertex_data[vertex_offset+14] = -1.0f;

				vertex_data[vertex_offset+15] = 1.0f;
				vertex_data[vertex_offset+16] = 1.0f;
				vertex_data[vertex_offset+17] = 1.0f;
				vertex_data[vertex_offset+18] = 1.0f;
			}
			
			vertex_offset += VERTEX_LENGTH;
		}

		if(scene->mNumAnimations > 0)
		{
			for(u32 i = 0; i < m->mNumBones; ++i)
			{
				//Find the node that is related to the anim bone and store the animation data
				cwsNode *node = NULL;
				node = find_node_by_name(&mesh->anim_data->root_node, (const char *)m->mBones[i]->mName.data);
				if(node != NULL && node->anim_bone == NULL)
				{
					aiMatrix4x4 mat = m->mBones[i]->mOffsetMatrix;
					node->anim_bone = malloc(sizeof(cwsAnimBone));
					node->anim_bone->offset_matrix = (mat4){.m[0] = mat.a1, .m[1] = mat.b1, .m[2] = mat.c1, .m[3] = mat.d1,
															.m[4] = mat.a2, .m[5] = mat.b2, .m[6] = mat.c2, .m[7] = mat.d2,
															.m[8] = mat.a3, .m[9] = mat.b3, .m[10] = mat.c3, .m[11] = mat.d3,
															.m[12] = mat.a4, .m[13] = mat.b4, .m[14] = mat.c4, .m[15] = mat.d4};
					node->anim_bone->gpu_name = NULL;
				}

				//Set vertex weights
				for(u32 j = 0; j < m->mBones[i]->mNumWeights; ++j)
				{
					//Vertex index (add vertex_index_offset as we combine all the meshes into one single mesh - Assimp stores indices as local and we ncwsd them as global)
					i32 vid = (vertex_index_offset + m->mBones[i]->mWeights[j].mVertexId)*VERTEX_LENGTH;
					f32 weight = m->mBones[i]->mWeights[j].mWeight;

					for(i32 k = 0; k < 4; ++k)
					{
						if(weight > 0 && vertex_data[vid+12+k] == -1)
						{
							vertex_data[vid+12+k] = i+bone_index_offset;
							vertex_data[vid+16+k] = weight;
							break;
						}
					}
				}
			}
			bone_index_offset += m->mNumBones;

			//Load animations
			mesh->anim_data->animations = malloc(sizeof(cwsAnimation*)*scene->mNumAnimations);
			if(mesh->anim_data->animations == NULL)
			{
				cws_log("Error allocating memory for mesh->anim_data->animations!");
				return false;
			}
			mesh->anim_data->animations_count = scene->mNumAnimations;	

			for(i32 j = 0; j < mesh->anim_data->animations_count; ++j)
			{
				mesh->anim_data->animations[j] = malloc(sizeof(cwsAnimation));
				mesh->anim_data->animations[j]->duration = scene->mAnimations[j]->mDuration;
				mesh->anim_data->animations[j]->ticks_per_second = scene->mAnimations[j]->mTicksPerSecond;
				mesh->anim_data->animations[j]->channels = malloc(sizeof(cwsAnimNode*)*scene->mAnimations[j]->mNumChannels);
				mesh->anim_data->animations[j]->frame_factor = 1;
				mesh->anim_data->animations[j]->selected_clip = NULL;
				mesh->anim_data->animations[j]->clips_size = 0;
				mesh->anim_data->animations[j]->clips_count = 0;
				mesh->anim_data->animations[j]->clips = NULL;
				if(mesh->anim_data->animations[j]->channels == NULL)
				{
					cws_log("Error allocating memory for mesh->anim_data->animations[j].channels!");
					return false;
				}
				mesh->anim_data->animations[j]->channels_count = scene->mAnimations[j]->mNumChannels;

				//Get channel data
				for(u32 k = 0; k < scene->mAnimations[j]->mNumChannels; ++k)
				{
					mesh->anim_data->animations[j]->channels[k] = malloc(sizeof(cwsAnimNode));
					mesh->anim_data->animations[j]->channels[k]->name = copy_conststr((const char *)scene->mAnimations[j]->mChannels[k]->mNodeName.data, scene->mAnimations[j]->mChannels[k]->mNodeName.length);

					mesh->anim_data->animations[j]->channels[k]->positions = (vec3*)malloc(sizeof(vec3)*scene->mAnimations[j]->mChannels[k]->mNumPositionKeys);
					mesh->anim_data->animations[j]->channels[k]->rotations = (quat*)malloc(sizeof(quat)*scene->mAnimations[j]->mChannels[k]->mNumRotationKeys);
					mesh->anim_data->animations[j]->channels[k]->scalings =  (vec3*)malloc(sizeof(vec3)*scene->mAnimations[j]->mChannels[k]->mNumScalingKeys);

					mesh->anim_data->animations[j]->channels[k]->ptime = (f32*)malloc(sizeof(f32)*scene->mAnimations[j]->mChannels[k]->mNumPositionKeys);
					mesh->anim_data->animations[j]->channels[k]->rtime = (f32*)malloc(sizeof(f32)*scene->mAnimations[j]->mChannels[k]->mNumRotationKeys);
					mesh->anim_data->animations[j]->channels[k]->stime = (f32*)malloc(sizeof(f32)*scene->mAnimations[j]->mChannels[k]->mNumScalingKeys);

					mesh->anim_data->animations[j]->channels[k]->positions_count = scene->mAnimations[j]->mChannels[k]->mNumPositionKeys;
					mesh->anim_data->animations[j]->channels[k]->rotations_count = scene->mAnimations[j]->mChannels[k]->mNumRotationKeys;
					mesh->anim_data->animations[j]->channels[k]->scalings_count = scene->mAnimations[j]->mChannels[k]->mNumScalingKeys;

					for(i32 b = 0; b < mesh->anim_data->animations[j]->channels[k]->positions_count; ++b)
					{
						mesh->anim_data->animations[j]->channels[k]->positions[b] = 	(vec3){.x = scene->mAnimations[j]->mChannels[k]->mPositionKeys[b].mValue.x,
																							   .y = scene->mAnimations[j]->mChannels[k]->mPositionKeys[b].mValue.y,
																						 	   .z = scene->mAnimations[j]->mChannels[k]->mPositionKeys[b].mValue.z};
						mesh->anim_data->animations[j]->channels[k]->ptime[b] = scene->mAnimations[j]->mChannels[k]->mPositionKeys[b].mTime;
					}

					for(i32 b = 0; b < mesh->anim_data->animations[j]->channels[k]->scalings_count; ++b)
					{
						mesh->anim_data->animations[j]->channels[k]->scalings[b] = 		(vec3){.x = scene->mAnimations[j]->mChannels[k]->mScalingKeys[b].mValue.x,
																							   .y = scene->mAnimations[j]->mChannels[k]->mScalingKeys[b].mValue.y,
																						 	   .z = scene->mAnimations[j]->mChannels[k]->mScalingKeys[b].mValue.z};
						mesh->anim_data->animations[j]->channels[k]->stime[b] = scene->mAnimations[j]->mChannels[k]->mScalingKeys[b].mTime;
					}

					for(i32 b = 0; b < mesh->anim_data->animations[j]->channels[k]->rotations_count; ++b)
					{
						mesh->anim_data->animations[j]->channels[k]->rotations[b] = 	(quat){.x = scene->mAnimations[j]->mChannels[k]->mRotationKeys[b].mValue.x,
																					 	   	   .y = scene->mAnimations[j]->mChannels[k]->mRotationKeys[b].mValue.y,
																				 	   		   .z = scene->mAnimations[j]->mChannels[k]->mRotationKeys[b].mValue.z,
																					 	   	   .w = scene->mAnimations[j]->mChannels[k]->mRotationKeys[b].mValue.w};
						mesh->anim_data->animations[j]->channels[k]->rtime[b] = scene->mAnimations[j]->mChannels[k]->mRotationKeys[b].mTime;
					}
				}
			}
			/*
				Pre-fetch the Nodes->current_anim_node in order to save a Anim_Node lookup every animation frame
			*/
			for(i32 j = 0; j < mesh->anim_data->animations[0]->channels_count; ++j)
			{
				set_node_animnode(&mesh->anim_data->root_node, mesh->anim_data->animations[0]->channels[j]);
			}
		}
	}

	mesh->minB = min;
	mesh->maxB = max;

	//Fill the mesh
	cwsFillMesh(mesh, vertex_data, vertex_size, index_data, index_size);

	free(vertex_data);
	free(index_data);

	aiReleaseImport(scene);
	return true;
}

void cwsEmptyMesh(cwsMesh *mesh, i32 *vertex_attribs, i32 count)
{   
    glGenBuffers(2, mesh->buffers);
	glGenVertexArrays(1, &mesh->vao);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);

	mesh->minB = (vec3){.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX};
	mesh->maxB = (vec3){.x = FLT_MIN, .y = FLT_MIN, .z = FLT_MIN};
	mesh->anim_data = NULL;
	mesh->_icount = 0;

	//Enable attrib slots & get vertex sum
	i32 sum = 0;
	for(i32 j = 0; j < count; ++j)
	{
		glEnableVertexAttribArray(j);
		sum += vertex_attribs[j];
	}

	//Tell OpenGL the size of each vertex & its components
	i32 off = 0;
	for(i32 j = 0; j < count; ++j)
	{
		glVertexAttribPointer(j, vertex_attribs[j], GL_FLOAT, GL_FALSE, sizeof(f32)*sum, (GLvoid*)(sizeof(f32)*off));
		off += vertex_attribs[j];
	}

	glBindVertexArray(0);
}

void cwsPlaneMesh(cwsMesh *mesh, f32 uv_repeat)
{
	f32 plane_v[] = {
		-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1,
		-0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, uv_repeat, 1, 1, 1,
		 0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, uv_repeat, uv_repeat, 1, 1, 1,
		 0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, uv_repeat, 0.0f, 1, 1, 1
	};

	i32 plane_i[] = {
		0, 1, 2,
		2, 3, 0
	};

	i32 vertex_attribs[4] = {3, 3, 2, 3};
	i32 VERTEX_LENGTH = 11;
	cwsEmptyMesh(mesh, vertex_attribs, 4);
	mesh->minB = (vec3){.x = -0.5f, .y = -0.5f, .z = -0.5f};
	mesh->maxB = (vec3){.x = 0.5f,  .y = 0.5f,  .z = 0.5f};
	cwsFillMesh(mesh, plane_v, VERTEX_LENGTH*4, plane_i, 6);
}

void cwsCubeMesh(cwsMesh *mesh)
{
 	f32 cube_v[] = {
		-0.5f, -0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, 1, 1,
		-0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1,
		 0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1, 1, 1,
		 0.5f, -0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1, 1, 1,

		-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, 1, 1,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1,
		 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1, 1, 1,
		 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1, 1, 1
	};

	i32 cube_i[] = {
		0, 1, 2,
		2, 3, 0,

		4, 7, 6,
		6, 5, 4,

		4, 5, 1,
		1, 0, 4,

		7, 6, 2,
		2, 3, 7,

		//top
		4, 0, 3,
		3, 7, 4,

		//bot
		1, 5, 6,
		6, 2, 1
	};

	i32 vertex_attribs[4] = {3, 3, 2, 3};
	i32 VERTEX_LENGTH = 11;
	cwsEmptyMesh(mesh, vertex_attribs, 4);
	mesh->minB = (vec3){.x = -0.5f, .y = -0.5f, .z = -0.5f};
	mesh->maxB = (vec3){.x = 0.5f,  .y = 0.5f,  .z = 0.5f};
	cwsFillMesh(mesh, cube_v, VERTEX_LENGTH*8, cube_i, 36);
}

void cwsBoundMesh(cwsMesh *mesh)
{
 	f32 cube_v[] = {
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1, 1, 1,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1,
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1, 1, 1,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1, 1,

		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1, 1, 1,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1,
		1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1, 1, 1,
		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1, 1
	};

	i32 cube_i[] = {
		0, 1, 2,
		2, 3, 0,

		4, 5, 6,
		6, 7, 4,

		4, 5, 1,
		1, 0, 4,

		7, 6, 2,
		2, 3, 7,

		4, 0, 3,
		3, 7, 4,

		1, 5, 6,
		6, 2, 1
	};

	i32 vertex_attribs[4] = {3, 3, 2, 3};
	i32 VERTEX_LENGTH = 11;
	cwsEmptyMesh(mesh, vertex_attribs, 4);
	mesh->minB = (vec3){.x = -0.5f, .y = -0.5f, .z = -0.5f};
	mesh->maxB = (vec3){.x = 0.5f,  .y = 0.5f,  .z = 0.5f};
	cwsFillMesh(mesh, cube_v, VERTEX_LENGTH*8, cube_i, 36);
}

void delete_node_tree(cwsNode *n)
{
	for(i32 i = 0; i < n->children_count; ++i)
	{
		delete_node_tree(&n->children[i]);
	}

	if(n != NULL)
	{
		free(n->name);
		free(n->children);
		if(n->anim_bone != NULL)
		{
			free(n->anim_bone->gpu_name);
			free(n->anim_bone);
		}
	}
}

void cwsDeleteMesh(cwsMesh *mesh)
{
	if(mesh == NULL)
	{
		return;
	}

	glDeleteBuffers(2, &mesh->buffers[0]);
	glDeleteVertexArrays(1, &mesh->vao);

	mesh->vao = -1;
	mesh->buffers[0] = -1;
	mesh->buffers[1] = -1;

	if(mesh->anim_data != NULL)
	{
		for(i32 i = 0; i < mesh->anim_data->animations_count; ++i)
		{
			for(i32 j = 0; j < mesh->anim_data->animations[i]->channels_count; ++j)
			{
				free(mesh->anim_data->animations[i]->channels[j]->positions);
				free(mesh->anim_data->animations[i]->channels[j]->rotations);
				free(mesh->anim_data->animations[i]->channels[j]->scalings);

				free(mesh->anim_data->animations[i]->channels[j]->ptime);
				free(mesh->anim_data->animations[i]->channels[j]->rtime);
				free(mesh->anim_data->animations[i]->channels[j]->stime);

				mesh->anim_data->animations[i]->channels[j]->positions_count = 0;
				mesh->anim_data->animations[i]->channels[j]->rotations_count = 0;
				mesh->anim_data->animations[i]->channels[j]->scalings_count = 0;

				free(mesh->anim_data->animations[i]->channels[j]->name);
				free(mesh->anim_data->animations[i]->channels[j]);
			}

			free(mesh->anim_data->animations[i]->channels);
			mesh->anim_data->animations[i]->channels_count = 0;

			free(mesh->anim_data->animations[i]);
		}

		free(mesh->anim_data->animations);
		mesh->anim_data->animations_count = 0;

		delete_node_tree(&mesh->anim_data->root_node);
		free(mesh->anim_data);
	}
}

void cwsClearMesh(cwsMesh *mesh)
{
	if(mesh == NULL)
	{
		return;
	}

	cwsFillMesh(mesh, NULL, 0, NULL, 0);
}

void cwsFillMesh(cwsMesh *mesh, f32 *vertex_data, i32 vcount, i32 *index_data, i32 icount)
{
	if(mesh == NULL)
	{
		return;
	}

	mesh->_icount = icount;
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vcount * sizeof(f32), vertex_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, icount * sizeof(i32), index_data, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void video_fill_submesh(cwsMesh *mesh, f32 *vertex_data, i32 voffset, i32 vlength, i32 *index_data, i32 ioffset, i32 ilength)
{
	if(mesh == NULL)
	{
		return;
	}
		
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(f32)*voffset, sizeof(f32)*vlength, vertex_data);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32)*ioffset, sizeof(i32)*ilength, index_data);
}

bool cwsTextureFromfile(cwsTexture2D *tex, const char *file, i32 filter)
{
	SDL_Surface *img = IMG_Load(file);
	if(img == NULL)
	{
		cws_log("Couldn't load image %s", file);
		return false;
	}

    cwsTextureFromsrc(tex, img, filter);
	SDL_FreeSurface(img);
    return true;
}

bool cwsTextureFromsrc(cwsTexture2D *tex, SDL_Surface *img, i32 filter)
{
	//Color mode
	u32 mode = GL_RGB;
	u32 internal = GL_RGB;
	if(img->format->BytesPerPixel == 1)
	{
		mode = GL_RED;
		internal = GL_RED;
	}
	else if(img->format->BytesPerPixel == 2)
	{
		mode = GL_RG;
		internal = GL_RG;
	}
	else if(img->format->BytesPerPixel == 4)
	{
		mode = GL_RGBA;
		internal = GL_RGBA;
	}

	//Pixels are stored reversed
	if(img->format->Rmask == 0x0000ff00)
	{
		if(mode == GL_RGB)
		{
			mode = GL_BGR;
		}
		else if(mode == GL_RGBA)
		{
			mode = GL_BGRA;
		}
	}

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filters[filter]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filters[filter]);
	glTexImage2D(GL_TEXTURE_2D, 0, internal, img->w, img->h, 0, mode, GL_UNSIGNED_BYTE, img->pixels);
    if(filter > 1)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
	glBindTexture(GL_TEXTURE_2D, 0);

	tex->size = (ivec2){.x = img->w, .y = img->h};
	return true;
}

void cwsDeleteTexture(cwsTexture2D *tex)
{
    glDeleteTextures(1, &tex->id);
    tex->id = 0;
}

void cwsTexture2DArrayAppend(cwsTexture2DArray *a, cwsImage2D *i)
{
	if(i->size.x > a->size.x)
		a->size.x = i->size.x;
	if(i->size.y > a->size.y)
		a->size.y = i->size.y;
	if(a->images == NULL)
	{
		a->images = (cwsImage2D**)malloc(sizeof(cwsImage2D*));
		if(a->images == NULL)
		{
			cws_log("Error allocating cwsTexture2DArray->images!");
			return;
		}

		a->images_size = 1;
		a->images_count = 1;
		a->images[0] = i;
	}
	else if(a->images_count >= a->images_size)
	{
		cwsImage2D **d = (cwsImage2D**)realloc(a->images, sizeof(cwsImage2D*)*(a->images_size+1));
		if(d == NULL)
		{
			cws_log("Error allocating more space for cwsTexture2DArray->images!");
			return;
		}

		a->images = d;
		a->images_size += 1;
		a->images[a->images_count++] = i;
	}
	else
	{
		a->images[a->images_count++] = i;	
	}
}

void cwsTexture2DArrayUpdate(cwsTexture2DArray *a)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, a->id);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, a->size.x, a->size.y, a->images_count);
	for(u32 i = 0; i < a->images_count; ++i)
	{
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, a->images[i]->size.x, a->images[i]->size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, a->images[i]->pixels);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void cwsDeleteTexture2DArray(cwsTexture2DArray *a)
{
    glDeleteTextures(1, &a->id);
    a->id = 0;
    a->size = (ivec2){.x = 0, .y = 0};
}

bool cwsImageFromfile(cwsImage2D *img, const char *file)
{
	SDL_Surface *s = IMG_Load(file);
	if(!s)
	{
		cws_log("%s", IMG_GetError());
		return NULL;
	}

	SDL_PixelFormat *fmt = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
	fmt->BitsPerPixel = 32;
	fmt->BytesPerPixel = 4;
	fmt->Rmask = s->format->Rmask;
	fmt->Gmask = s->format->Gmask;
	fmt->Bmask = s->format->Bmask;
	fmt->Amask = s->format->Amask;

	SDL_Surface *c = SDL_ConvertSurface(s, fmt, 0);

	u32 *mem = (u32*)c->pixels;
	img->pixels = (u32*)malloc(sizeof(u32) * (c->w*c->h));
	for(i32 i = 0; i < c->w*c->h; ++i)
	{
		img->pixels[i] = mem[i];
	}

	img->size = (ivec2){.x = c->w, .y = c->h};
	SDL_FreeSurface(c);
	SDL_FreeSurface(s);

	free(fmt);
    return true;
}

void cwsDeleteImage(cwsImage2D *im)
{
    free(im->pixels);
    im->pixels = NULL;
}

bool cwsShaderFromsrc(cwsShader* shader, const char *vertex_src, const char *fragment_src)
{
	shader->id = glCreateProgram();

	if(vertex_src != NULL && fragment_src != NULL)
	{
		i32 vid = glCreateShader(GL_VERTEX_SHADER);
		i32 fid = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vid, 1, (const char**)&vertex_src, NULL);
		glShaderSource(fid, 1, (const char**)&fragment_src, NULL);

		glCompileShader(vid);

		char infoLog[1024];
		i32 infoLength = 0;
		glGetShaderInfoLog(vid, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("Vertex shader error:\n%s\n", infoLog);
			return false;
		}
		infoLength = 0;
	
		glCompileShader(fid);
		glGetShaderInfoLog(fid, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("Fragment shader error:\n%s\n", infoLog);
			return false;
		}		
		infoLength = 0;

		glAttachShader(shader->id, vid);
		glAttachShader(shader->id, fid);
		glLinkProgram(shader->id);

		glGetProgramInfoLog(shader->id, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("Shader program error:\n%s\n", infoLog);
			return false;
		}

		glDetachShader(shader->id, vid);
		glDetachShader(shader->id, fid);
		glDeleteShader(vid);
		glDeleteShader(fid);
	}

	shader->mvp_id = glGetUniformLocation(shader->id, "mvp_matrix");
	shader->model_id = glGetUniformLocation(shader->id, "model_matrix");
	shader->view_id = glGetUniformLocation(shader->id, "view_matrix");
	shader->projection_id = glGetUniformLocation(shader->id, "projection_matrix");
    return true;
}

void cwsShaderCreateUniform(cwsShader *s, const char *name)
{
    cws_array_push(s->uniforms, glGetUniformLocation(s->id, name));
    cws_string st = cws_string();
    cws_string_build(&st, name);
    cws_array_push(s->unames, st);
}

void cwsShaderBufferUniform(cwsShader *s, const char *name, f32 *values, i32 length)
{
    u32 id = -1;
    for(u32 i = 0; i < s->unames.length; ++i)
    {
        if(strcmp((const char *)s->unames.data[i].data, name) == 0)
        {
            id = s->uniforms.data[i];
            break;
        }
    }
    
    if(id >= 0)
    {
        switch(length)
        {
            case 1:
            glUniform1f(id, values[0]);
            break;
            case 2:
            glUniform2f(id, values[0], values[1]);
            break;
            case 3:
            glUniform3f(id, values[0], values[1], values[2]);
            break;
            case 4: 
            glUniform4f(id, values[0], values[1], values[2], values[3]);
            break;
            case 9:
            glUniformMatrix3fv(id, 1, false, values);
            break;
            case 16:
            glUniformMatrix4fv(id, 1, false, values);
            break;
        }
    }
}

const char *default_fheader =
"#version 330\n"
"uniform mat4 model_matrix, view_matrix, projection_matrix, mvp_matrix;"
"in vec2 Uv;"
"in vec3 Color;"
"in vec3 Normal;"
"in vec3 Pos;"
"out vec4 FinalColor;";

const char *default_vheader =
"#version 330\n"
"uniform mat4 model_matrix, view_matrix, projection_matrix, mvp_matrix;"
"out vec2 Uv;"
"out vec3 Color;"
"out vec3 Normal;"
"out vec3 Pos;";

const char *lighting_fheader =
"layout (std140) uniform LightData"
"{"
    "vec4 ambient_light;"
    "vec4 lights_count;"
    "mat4 shadow_matrices_lod[4];"
    "mat4 spotlight_matrices[8];"
    "vec4 dirlights_dir[2];"
    "vec4 dirlights_color[2];"
    "vec4 pointlights_pos[16];"
    "vec4 pointlights_intensity[16];"
    "vec4 spotlights_pos[16];"
    "vec4 spotlights_dir[16];"
    "vec4 spotlights_intensity[16];"
    "vec4 spotlights_conedata[16];"
"};"
"vec3 cwsLightingColor(vec3 pos, vec3 normal)"
"{"
"vec3 c = vec3(0,0,0);"
"for(int i = 0; i < lights_count.x; ++i){"
	"vec3 p = -dirlights_dir[i].xyz;"
	"vec3 n = normal;"

    "c += (dirlights_color[i].xyz * max(dot(n,p), 0.0f)) * ambient_light.xyz;"
"}"
"for(int i = 0; i < lights_count.y; ++i){"
    "vec3 p = pointlights_pos[i].xyz-pos;"
    "vec3 n = normal;"
    "float d = distance(pointlights_pos[i].xyz,pos);"
    "float att = pointlights_intensity[i].w / (1.0f + 0.1f*d + 0.01f*(d*d));"
    "c += pointlights_intensity[i].xyz * att * clamp(dot(n,p) / (length(p) * length(n)), 0.0, 1.0);"
"}"
"for(int i = 0; i < lights_count.z; ++i){"
    "vec3 p = spotlights_pos[i].xyz-pos;"
	"vec3 n = normal;"
	"float d = distance(spotlights_pos[i].xyz,pos);"
	"float att = clamp((spotlights_intensity[i].w / (1.0f + 0.1f*d + 0.01f*(d*d))), 0.0f, 2.0f);"
	"float inner_cutoff = spotlights_conedata[i].x;"
	"float outer_cutoff = spotlights_conedata[i].x - spotlights_conedata[i].y;"
	"float cos_angle = dot(normalize(-spotlights_dir[i].xyz), normalize(p));"
	"float falloff = clamp((cos_angle - outer_cutoff) / (inner_cutoff-outer_cutoff), 0.0f, 1.0f);"
    "c += (spotlights_intensity[i].xyz * clamp((dot(n,p) / (length(p) * length(n))), 0, 1)) * falloff * att;"
"}"
"return clamp(c, 0.0f, 1.0f);"
"}";

const char *lighting_vheader = 
"layout (std140) uniform LightData"
"{"
"vec4 ambient_light;"
"vec4 lights_count;"
"mat4 shadow_matrices_lod[4];"
"mat4 spotlight_matrices[8];"
"vec4 dirlights_dir[2];"
"vec4 dirlights_color[2];"
"vec4 pointlights_pos[16];"
"vec4 pointlights_intensity[16];"
"vec4 spotlights_pos[16];"
"vec4 spotlights_dir[16];"
"vec4 spotlights_intensity[16];"
"vec4 spotlights_conedata[16];"
"};";

const char *shadow_vheader = 
"out vec4 shadow_lightspace_pos_lod1;"
"out vec4 shadow_lightspace_pos_lod2;"
"out vec4 shadow_lightspace_pos_lod3;"
"out vec4 shadow_lightspace_pos_lod4;"
"out vec4 EE_spotlights_lightspace_pos[8];"
"out vec4 P_Pos;"
"void cwsShadowPass(vec3 pos, vec3 Normal)"
"{"
    "shadow_lightspace_pos_lod1 = shadow_matrices_lod[0] * model_matrix * vec4(pos+Normal*0.005f,1.0f);"
	"shadow_lightspace_pos_lod2 = shadow_matrices_lod[1] * model_matrix * vec4(pos+Normal*0.005f,1.0f);"
	"shadow_lightspace_pos_lod3 = shadow_matrices_lod[2] * model_matrix * vec4(pos+Normal*0.005f,1.0f);"
	"shadow_lightspace_pos_lod4 = shadow_matrices_lod[3] * model_matrix * vec4(pos+Normal*0.005f,1.0f);"

	"for(int i = 0; i < lights_count.z; ++i)"
	"{"
		"EE_spotlights_lightspace_pos[i] = spotlight_matrices[i] * model_matrix * vec4(pos,1.0f);"
	"}"
"}";

const char *shadow_fheader = 
"uniform sampler2D shadow_map;"
"uniform sampler2D EE_spotlights_shadow_map[8];"
"in vec4 shadow_lightspace_pos_lod1;"
"in vec4 shadow_lightspace_pos_lod2;"
"in vec4 shadow_lightspace_pos_lod3;"
"in vec4 shadow_lightspace_pos_lod4;"
"in vec4 EE_spotlights_lightspace_pos[8];"
"in vec4 P_Pos;"
"float sample_dirlight_shadow(int index, vec4 l_pos, sampler2D map, float filter_steps, vec2 uv_offset)"
"{"
	"vec4 lightspace_pos = l_pos;"
	"vec3 pcoords = lightspace_pos.xyz / lightspace_pos.w;"

	"float a = 1.0f / 2048.0f;"
	"pcoords.x = floor(pcoords.x*2048)*a;"
	"pcoords.y = floor(pcoords.y*2048)*a;"

	"vec2 uvc;"
	"uvc.x = 0.25f * pcoords.x + 0.25f + uv_offset.x;"
	"uvc.y = 0.25f * pcoords.y + 0.25f + uv_offset.y;"
	"float z = 0.5f * pcoords.z + 0.5f;"

	"vec3 p = -dirlights_dir[index].xyz;"
	"vec3 n = Normal;"

	"float res = 0.0f;"

	"filter_steps *= 1.0f - float(int(z));"
	"for(float x = -filter_steps; x <= filter_steps; ++x)"
	"{"
		"for(float y = -filter_steps; y <= filter_steps; ++y)"
		"{"
			"vec2 off = vec2(x,y)*vec2(a,a);"
			"float depth = texture(map, uvc+off).r;"
			"if(depth < z)"
			"{"
				"res += 1.0f;"
			"}"
		"}"
	"}"
	
	"res /= (filter_steps+filter_steps)*(filter_steps+filter_steps);"
	"res = clamp(res, 0.0f, 0.25f);"

	"return res;"
"}"
"float cwsShadowFactor(vec3 fpos)"
"{"
	/*
	    Sample shadows for the directional light
        Frustum splits are chosen depending on the bounds of a sphere as our frustum splits are fit inside a sphere and not a box.
		Sphere eq = x^2+y^2+z^2 = r^2
     */
	"float factor = 0.0f;"
	"float P_Pos_sqr = (P_Pos.x*P_Pos.x)+(P_Pos.y*P_Pos.y)+(P_Pos.z*P_Pos.z);"

	"float asd = clamp(P_Pos_sqr, 0.0f, 4096.0f);"
	"int ind = clamp(int(step(asd,63) + step(asd,255) + step(asd,1023)), 0, 3);"

	"vec4 dirlight_lsp[4]; "
	"dirlight_lsp[0] = shadow_lightspace_pos_lod4;"
	"dirlight_lsp[1] = shadow_lightspace_pos_lod3;"
	"dirlight_lsp[2] = shadow_lightspace_pos_lod2;"
	"dirlight_lsp[3] = shadow_lightspace_pos_lod1;"

	"vec2 cascade_offsets[4];"
	"cascade_offsets[0] = vec2(0.5,0.5);"
	"cascade_offsets[1] = vec2(0.0,0.5);"
	"cascade_offsets[2] = vec2(0.5,0.0);"
	"cascade_offsets[3] = vec2(0.0,0.0);"

	"factor += sample_dirlight_shadow(0, dirlight_lsp[ind], shadow_map, 1, cascade_offsets[ind]);"

    /*
        Sample shadows for spot lights
    */
"for(int i = 0; i < lights_count.z; ++i)"
"{"
		"vec4 lightspace_pos = EE_spotlights_lightspace_pos[i];"
		"vec3 pcoords = lightspace_pos.xyz / lightspace_pos.w;"
		"vec2 uvc;"
		"uvc.x = 0.5f * pcoords.x + 0.5f;"
		"uvc.y = 0.5f * pcoords.y + 0.5f;"
		"float z = (pcoords.z + 1.0f) + 1.0f;"

		"vec3 p = spotlights_pos[i].xyz-fpos;"
		"vec3 n = normalize(Normal);"
		"float d = distance(spotlights_pos[i].xyz,fpos);"
		"float att = (spotlights_intensity[i].w*0.25f / (1.0f + 0.1f*d + 0.01f*(d*d)));"

		"float inner_cutoff = spotlights_conedata[i].x;"
		"float outer_cutoff = spotlights_conedata[i].x - spotlights_conedata[i].y;"
		"float cos_angle = dot(normalize(-spotlights_dir[i].xyz), normalize(p));"

		"float falloff = clamp((cos_angle - outer_cutoff) / (inner_cutoff-outer_cutoff), 0.0f, 1.0f);"
		"float intensity = clamp((dot(n,p) / (length(p) * length(n))), 0, 1);"

		"z = clamp(z, 0.0f, 1.0f);"

		"float res = 0.0f;"
		"for(int x = -1; x <= 1; ++x)"
		"{"
			"for(int y = -1; y <= 1; ++y)"
			"{"
				"vec2 off = vec2(x,y) / vec2(1024,1024);"
			"}"
		"}"
		"res /= 9.0f;"
		"factor += res;"
"}"
    "return (1.0f - clamp(factor, 0.0f, 1.0f));"
"}";

bool cwsShaderFromfile(cwsShader *shader, const char *vertex_file, const char *frag_file, i32 hflags)
{
	shader->id = glCreateProgram();

	i32 vlength = 0, flength = 0;
	char *vertex = cwsReadFile(vertex_file, &vlength);
	char *fragment = cwsReadFile(frag_file, &flength);

	if(vertex != NULL && fragment != NULL)
	{
        //Calculate size of full shader program including any precreated headers
        
        //Vertex shader
        i32 vfull_length = vlength +
            (((hflags & SH_DEFAULT) != 0) ? (i32)strlen(default_vheader) : 0) +
            (((hflags & SH_LIGHTING) != 0) ? (i32)strlen(lighting_vheader) : 0) +
            (((hflags & SH_SHADOWS) != 0) ? (i32)strlen(shadow_vheader) : 0);
        
        char *full_vert = malloc(sizeof(char) * vfull_length); 
        
        i32 voffset = 0;
       
        if(hflags & SH_DEFAULT)
        { 
            strncpy(full_vert, default_vheader, (i32)strlen(default_vheader));
            voffset += strlen(default_vheader);
        }
        
        if(hflags & SH_LIGHTING)
        {
            strncpy(full_vert+voffset, lighting_vheader, (i32)strlen(lighting_vheader));
            voffset += strlen(lighting_vheader);
        }
        
        if(hflags & SH_SHADOWS)
        {
            strncpy(full_vert+voffset, shadow_vheader, (i32)strlen(shadow_vheader));
            voffset += strlen(shadow_vheader);
        }
        
        strcpy(full_vert+(sizeof(char)*voffset), vertex);
        
        //Fragment shader
        i32 full_length = flength +
            (((hflags & SH_DEFAULT) != 0) ? strlen(default_fheader) : 0) +
            (((hflags & SH_LIGHTING) != 0) ? (i32)strlen(lighting_fheader) : 0) +
            (((hflags & SH_SHADOWS) != 0) ? (i32)strlen(shadow_fheader) : 0);
        char *full_frag = malloc(sizeof(char) * full_length); 
        
        i32 offset = 0;
        
        if(hflags & SH_DEFAULT)
        {
            strncpy(full_frag, default_fheader, (i32)strlen(default_fheader));
            offset += strlen(default_fheader);
        }
        
        if(hflags & SH_LIGHTING)
        {
            strncpy(full_frag+offset, lighting_fheader, (i32)strlen(lighting_fheader));
            offset += strlen(lighting_fheader);
        }
        
        if(hflags & SH_SHADOWS)
        {
            strncpy(full_frag+offset, shadow_fheader, (i32)strlen(shadow_fheader));
            offset += strlen(shadow_fheader);
        }
        
        strcpy(full_frag+(sizeof(char)*offset), fragment);
        i32 vid = glCreateShader(GL_VERTEX_SHADER);
		i32 fid = glCreateShader(GL_FRAGMENT_SHADER);
        
		glShaderSource(vid, 1, (const char**)&full_vert, &vfull_length);
        glShaderSource(fid, 1, (const char**)&full_frag, &full_length);

		glCompileShader(vid);

		char infoLog[1024];
		i32 infoLength = 0;
		glGetShaderInfoLog(vid, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("%s vertex shader error:\n%s\n", vertex_file, infoLog);
			return false;
		}
		infoLength = 0;
	
		glCompileShader(fid);
		glGetShaderInfoLog(fid, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("%s fragment shader error:\n%s\n", frag_file, infoLog);
			return false;
		}		
		infoLength = 0;

		glAttachShader(shader->id, vid);
		glAttachShader(shader->id, fid);
		glLinkProgram(shader->id);

		glGetProgramInfoLog(shader->id, 1024, &infoLength, infoLog);
		if(infoLength > 0)
		{
			cws_log("[%s] [%s] shader program error:\n%s\n", vertex_file, frag_file, infoLog);
			return false;
		}

		glDetachShader(shader->id, vid);
		glDetachShader(shader->id, fid);
		glDeleteShader(vid);
		glDeleteShader(fid);

        free(full_frag);
    }

	free(vertex);
	free(fragment);

	shader->mvp_id = glGetUniformLocation(shader->id, "mvp_matrix");
	shader->model_id = glGetUniformLocation(shader->id, "model_matrix");
	shader->view_id = glGetUniformLocation(shader->id, "view_matrix");
	shader->projection_id = glGetUniformLocation(shader->id, "projection_matrix");
	return true;
}

void cwsDeleteShader(cwsShader *shader)
{
    glDeleteProgram(shader->id);
    shader->id = 0;
    shader->mvp_id = 0;
    shader->view_id = 0;
    shader->model_id = 0;
    shader->projection_id = 0;
    cws_array_free(shader->unames);
    cws_array_free(shader->uniforms);
}

void cwsDeleteMaterial(cwsMaterial *mat)
{
    cws_array_free(mat->texture_uniforms);
    cws_array_free(mat->texture_array);
}

void cwsMaterialAddTexture(cwsMaterial *mat, cwsTexture2D tex)
{
	//Create a texture uniform which will be used to bind this texture
	char buf[5];
    sprintf(buf, "tex%d", mat->texture_array.length);
    cws_array_push(mat->texture_uniforms, glGetUniformLocation(mat->shader.id, buf));
    cws_array_push(mat->texture_array, tex);
}

void cwsBindMaterial(cwsMaterial *mat)
{
	active_material = mat;
	
    if(active_material->shader.id != 0)
    {
        if(!glIsProgram(active_material->shader.id))
        {
            cwsShaderInit(active_material->shader);
            cws_log("Error material used without a shader!");
        }
        
        glUseProgram(active_material->shader.id);
    }
    
	glUniform3f(glGetUniformLocation(active_material->shader.id, "in_color"), mat->color.x, mat->color.y, mat->color.z);

	if(active_material->texture_array.length > 0)
	{
		for(u32 i = 0; i < active_material->texture_array.length; ++i)
		{
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, active_material->texture_array.data[i].id);
			glUniform1i(active_material->texture_uniforms.data[i], i);
		}
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if((mat->rflags & RF_CULLING_ENABLED))
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if((mat->rflags & RF_WIREFRAME_ENABLED))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	}
    
    if((mat->rflags & RF_NO_DEPTH_TEST))
    {
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
    }
}

/*
	This function should be called from mesh_animate only!
*/
void mesh_traverse_animtree(cwsMesh *mesh, cwsNode *node, mat4 parent_transform)
{
	//mat4 node_transform = node->transform;
	mat4 node_transform = mat4_default;
	cwsAnimNode *n = node->current_anim_node;
	if(n)
	{
		f32 a_time = mesh->anim_data->animation_time;

		//INTERPOLATE ROTATION
		i32 index = 0;
		for(i32 i = 0; i < n->rotations_count-1; ++i)
		{
			if(a_time < n->rtime[i+1])
			{
				index = i;
				break;
			}
		}

		i32 next_index = index+1;

		f32 delta_time = n->rtime[next_index] - n->rtime[index];
		f32 factor = (f32)fabs((a_time - n->rtime[index])) / delta_time;
		quat start = n->rotations[index];
		quat end = n->rotations[next_index];

		quat q = quat_slerp(start, end, factor);
		q = quat_normalize(q);

		//INTERPOLATE TRANSLATION
		index = 0;
		for(i32 i = 0; i < n->positions_count-1; ++i)
		{
			if(a_time < n->ptime[i+1])
			{
				index = i;
				break;
			}
		}
		
		next_index = index+1;

		delta_time = n->ptime[next_index] - n->ptime[index];
		factor = (f32)fabs((a_time - n->ptime[index])) / delta_time;
		vec3 start_p = n->positions[index];
		vec3 end_p = n->positions[next_index];

		start_p = vec3_mul_scalar(start_p, factor);
		end_p = vec3_mul_scalar(end_p, 1 - factor);
		vec3 v = vec3_add(start_p, end_p);
		mat4 tr = mat4_translate(mat4_default, v);

		//INTERPOLATE SCALING
		index = 0;
		for(i32 i = 0; i < n->scalings_count-1; ++i)
		{
			if(a_time < n->stime[i+1])
			{
				index = i;
				break;
			}
		}

		next_index = index+1;

		delta_time = n->stime[next_index] - n->stime[index];
		factor = (f32)fabs((a_time - n->stime[index])) / delta_time;

		start_p = n->scalings[index];
		end_p = n->scalings[next_index];
		
		start_p = vec3_mul_scalar(start_p, factor);
		end_p = vec3_mul_scalar(end_p, 1 - factor);
		v = vec3_add(start_p, end_p);

		mat4 qmat = quat_to_mat4(q);
		mat4 ts = mat4_scale(mat4_default, v);
		node_transform = mat4_mul(tr,qmat);
		node_transform = mat4_mul(node_transform,ts);
	}

	parent_transform = mat4_mul(parent_transform,node_transform);

	if(node->anim_bone != NULL)
	{
		node->anim_bone->transform = mat4_mul(parent_transform,node->anim_bone->offset_matrix);
		node->anim_bone->transform = mat4_mul(mesh->anim_data->inverse_transform,node->anim_bone->transform);
	}

	for(i32 i = 0; i < node->children_count; ++i)
	{
		mesh_traverse_animtree(mesh, &node->children[i], parent_transform);	
	}
}

void mesh_animate(cwsMesh *mesh, f32 time_in_secs)
{
	if(mesh->anim_data == NULL || mesh->anim_data->animations[0]->selected_clip == NULL)
		return;

	f32 ticks_per_second = mesh->anim_data->animations[0]->ticks_per_second != 0 ? mesh->anim_data->animations[0]->ticks_per_second : 25.0f;
	f32 time_in_ticks = time_in_secs * ticks_per_second;

	f32 fpm = mesh->anim_data->animations[0]->frame_factor;
	f32 start = fpm * mesh->anim_data->animations[0]->selected_clip->start_frame;
	f32 end = fpm * mesh->anim_data->animations[0]->selected_clip->end_frame;
	mesh->anim_data->animation_time = fmod(time_in_ticks, end-start)+start;

	mat4 id = mat4_default;
	mesh_traverse_animtree(mesh, &mesh->anim_data->root_node, id);
}

void cwsBindMesh(cwsMesh *mesh)
{
	glBindVertexArray(mesh->vao);
}

void cwsDrawMesh(mat4 transform, cwsMesh *mesh, i32 p)
{
	if(mesh == NULL || active_material == NULL)
	{
		return;
	}

	mat4 mvp = mat4_mul(main_projection_matrix, main_view_matrix);
	mvp 	 = mat4_mul(mvp, transform);

	glUniformMatrix4fv(active_material->shader.model_id, 1, GL_FALSE, transform.m);
	glUniformMatrix4fv(active_material->shader.mvp_id, 1, GL_FALSE, mvp.m);
	glUniformMatrix4fv(active_material->shader.view_id, 1, GL_FALSE, main_view_matrix.m);
	glUniformMatrix4fv(active_material->shader.projection_id, 1, GL_FALSE, main_projection_matrix.m);
	glDrawElements(p, mesh->_icount, GL_UNSIGNED_INT, NULL);
}

//Buffers the bone matrix data and continues down the tree
void continue_bone_search(cwsNode *r, i32 *j);
void buffer_bone_uniforms(cwsNode *r, i32 *j)
{
	if(r->anim_bone->gpu_name == NULL)
	{
		r->anim_bone->gpu_name = malloc(sizeof(char)*32);
		sprintf(r->anim_bone->gpu_name, "EE_bone_matrices[%d]", *j);
	}

	(*j)++;
	glUniformMatrix4fv(glGetUniformLocation(active_material->shader.id, r->anim_bone->gpu_name), 1, GL_FALSE, r->anim_bone->transform.m);

	for(i32 i = 0; i < r->children_count; ++i)
	{
		if(r->children[i].anim_bone != NULL)
		{
			buffer_bone_uniforms(&r->children[i], j);
		}
		else
		{
			continue_bone_search(&r->children[i], j);
		}
	}
}

//Just continues the search until a animation bone is found in the node structure
void continue_bone_search(cwsNode *r, i32 *j)
{
	for(i32 i = 0; i < r->children_count; ++i)
	{
		if(r->children[i].anim_bone != NULL)
		{
			buffer_bone_uniforms(&r->children[i],j);
		}
		else
		{
			continue_bone_search(&r->children[i],j);
		}
	}
}

void cwsDrawAnimatedMesh(mat4 transform, cwsMesh *mesh, i32 p, f32 time_in_secs)
{
	if(mesh == NULL || active_material == NULL)
	{
		return;
	}

	/*
		Do mesh animation
	*/
	if(mesh->anim_data->animations[0] != NULL)
	{
		mesh_animate(mesh, time_in_secs);
		i32 j = 0;

		//Recursivly search through the node tree and send the bone matrices to the shader
		for(i32 i = 0; i < mesh->anim_data->root_node.children_count; ++i)
		{
			if(mesh->anim_data->root_node.children[i].anim_bone != NULL)
			{
				buffer_bone_uniforms(&mesh->anim_data->root_node.children[i],&j);
			}
			else
			{
				continue_bone_search(&mesh->anim_data->root_node.children[i],&j);
			}
		}
	}

	mat4 mvp = mat4_mul(main_projection_matrix, main_view_matrix);
	mvp 	 = mat4_mul(mvp, transform);

	glUniformMatrix4fv(active_material->shader.mvp_id, 1, GL_FALSE, mvp.m);
	glUniformMatrix4fv(active_material->shader.view_id, 1, GL_FALSE, main_view_matrix.m);
	glUniformMatrix4fv(active_material->shader.model_id, 1, GL_FALSE, transform.m);
	glUniformMatrix4fv(active_material->shader.projection_id, 1, GL_FALSE, main_projection_matrix.m);
	glDrawElements(p, mesh->_icount, GL_UNSIGNED_INT, NULL);
}

bool cwsCreateTextContext(cwsTextContext *context, const char *filename)
{
	i32 length = 0;
	char *buf = cwsReadFile(filename, &length);

	char read[64];
	i32 offset = 0;

	//Split the file into easy to read words
	cws_string* words = malloc(sizeof(cws_string)*32);
	if(words == NULL)
	{
		cws_log("Error allocating memory for words!");
		return false;
	}
	u32 words_size = 32;
	u32 words_count = 0;
	for(u32 i = 0; i < length; ++i)
	{
		if(buf[i] == '=' || buf[i] == '\"' || buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\t' || buf[i] == ',')
		{
			if(offset > 0)
			{
				read[offset++] = '\0';
				if(words_count >= words_size)
				{
					cws_string *nw = realloc(words, sizeof(cws_string)*(words_size+32));
					if(nw == NULL)
					{
						cws_log("Error allocating memory for words!");
						return NULL;
					}

					words = nw;
					words_size += 32;
				}

				words[words_count] = cws_string();
				cws_string_build(&words[words_count++], (const char *)read);
				offset = 0;
			}
		}
		else
		{
			read[offset] = buf[i];
			offset++;
		}
	}

	free(buf);

	context->texts = NULL;
	context->texts_size = 0;
	context->texts_count = 0;
	cws_string tfile = cws_string();
	for(u32 i = 0; i < FONT_ASCII_CHAR_COUNT; ++i)
	{
		context->chars[i] = (cwsFontChar){ .x = 0, .y = 0, .w = 0, .h = 0, .offset_x = 0, .offset_y = 0, .advance_x = 0};
	}

	for(u32 i = 0; i < words_count; ++i)
	{
		if(strcmp(words[i].data,"file") == 0)
		{
			cws_string_copy(&tfile,&words[i+1]);
		}
		else if(strcmp(words[i].data,"chars") == 0 && strcmp(words[i+1].data,"count") == 0)
		{
			i32 id = -1;
			for(u32 j = i+2; j < words_count; ++j)
			{
				if(strcmp(words[j].data,"char") == 0)
				{
					if(id > -1)
					{
						id = -1;
					}
				}
				else if(strcmp(words[j].data,"id") == 0)
				{
					id = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"x") == 0)
				{
					context->chars[id].x = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"y") == 0)
				{
					context->chars[id].y = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"width") == 0)
				{
					context->chars[id].w = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"height") == 0)
				{
					context->chars[id].h = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"xoffset") == 0)
				{
					context->chars[id].offset_x = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"yoffset") == 0)
				{
					context->chars[id].offset_y = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"xadvance") == 0)
				{
					context->chars[id].advance_x = atoi((const char *)words[j+1].data);
				}
				else if(strcmp(words[j].data,"kernings") == 0)
				{
					break;
				}
			}
		}
	}

	for(u32 i = 0; i < words_count; ++i)
	{
		cws_string_free(&words[i]);
	}

	free(words);

    context->texture = malloc(sizeof(cwsTexture2D));
    cwsTextureFromfile(context->texture, (const char *)tfile.data, IF_LINEAR);
	cws_string_free(&tfile);
	glGenBuffers(1, &context->tex_buffer);
	glGenTextures(1, &context->buffer_texture_id);

	i32 va[3] = {3,2,1};
    context->mesh = malloc(sizeof(cwsMesh));
    cwsEmptyMesh(context->mesh, va,3);
    return true;
}

void cwsDeleteTextContext(cwsTextContext *context)
{
	for(u32 j = 0; j < context->texts_count; ++j)
	{
		cws_string_free(&context->texts[j]->str);
		free(context->texts[j]);
	}	

    cwsDeleteTexture(context->texture);
    cwsDeleteMesh(context->mesh);
    free(context->mesh);
    free(context->texture);
	free(context->texts);
}

void cwsClearTextContext(cwsTextContext *context)
{
	if(context == NULL)
	{
		return;
	}

	cwsClearMesh(context->mesh);
	for(u32 i = 0; i < context->texts_count; ++i)
	{
		free(context->texts[i]);
	}
	free(context->texts);
}

void video_buffer_text_context(cwsTextContext *context)
{
    cws_array(f32, verts);
	cws_array_init(f32,verts,24);
    cws_array(i32, inds);
	cws_array_init(i32,inds,6);

	f32 tw = 1.0f / context->texture->size.x;
	f32 th = 1.0f / context->texture->size.y;
	i32 index_offset = 0;

	for(u32 j = 0; j < context->texts_count; ++j)
	{
		cwsText *t = context->texts[j];
		vec2 size = (vec2){.x = 0, .y = 0};
		f32 cursor = 0;
		f32 posX = 0;
		for(u32 i = 0; i < t->str.length; ++i)
		{
			i8 cv = (i8)t->str.data[i];

			//V1
			cws_array_push(verts, posX);
			cws_array_push(verts,context->chars[cv].offset_y);
			cws_array_push(verts,0);
			cws_array_push(verts,(f32)context->chars[cv].x * tw);
			cws_array_push(verts,(f32)context->chars[cv].y * th);
			cws_array_push(verts,j);

			//V2
			cws_array_push(verts,posX);
			cws_array_push(verts,context->chars[cv].offset_y + context->chars[cv].h);
			cws_array_push(verts,0);
			cws_array_push(verts,(f32)context->chars[cv].x * tw);
			cws_array_push(verts,(f32)(context->chars[cv].y + context->chars[cv].h) * th);
			cws_array_push(verts,j);
			
			//V3
			cws_array_push(verts,posX + context->chars[cv].w);
			cws_array_push(verts,context->chars[cv].offset_y + context->chars[cv].h);
			cws_array_push(verts,0);
			cws_array_push(verts,(f32)(context->chars[cv].x + context->chars[cv].w)*tw);
			cws_array_push(verts,(f32)(context->chars[cv].y + context->chars[cv].h) * th);
			cws_array_push(verts,j);

			//V4
			cws_array_push(verts,posX + context->chars[cv].w);
			cws_array_push(verts,context->chars[cv].offset_y);
			cws_array_push(verts,0);
			cws_array_push(verts,(f32)(context->chars[cv].x + context->chars[cv].w) * tw);
			cws_array_push(verts,(f32)context->chars[cv].y * th);
			cws_array_push(verts,j);

			//INDICES
			cws_array_push(inds,index_offset);
			cws_array_push(inds,index_offset+1);
			cws_array_push(inds,index_offset+2);

			cws_array_push(inds,index_offset+2);
			cws_array_push(inds,index_offset+3);
			cws_array_push(inds,index_offset);
			index_offset += 4;

			cursor += context->chars[cv].advance_x;
			posX = context->chars[cv].offset_x+cursor;

			size.x += context->chars[cv].advance_x;
			if(context->chars[cv].offset_y + context->chars[cv].h > size.y)
			{
				size.y = context->chars[cv].offset_y + context->chars[cv].h;
			}
		}
		
		t->__size = size;
	}

	cwsFillMesh(context->mesh, verts.data, verts.length, inds.data,inds.length);

	cws_array_free(verts);
	cws_array_free(inds);
}

cwsText* cwsNewText(cwsTextContext *context, vec2 pos, vec2 scale, const char *tx)
{
	if(context->texts == NULL)
	{
		context->texts = malloc(sizeof(cwsText*)*16);
		if(context->texts == NULL)
		{
			cws_log("Error allocating memory for context->texts!");
			return NULL;
		}

		context->texts_size = 16;
	}
	else if(context->texts_count >= context->texts_size)
	{
		cwsText** tls = realloc(context->texts, sizeof(cwsText*)*(context->texts_size+16));
		if(tls == NULL)
		{
			cws_log("Error reallocating memory for context-texts!");
			return NULL;
		}

		context->texts = tls;
		context->texts_size += 16;
	}

	cwsText *txt = malloc(sizeof(cwsText));
	txt->__size = (vec2){.x = 0, .y = 0};
	txt->bounds = (vec4){.x = 0, .y = 0, .z = 40, .w = 40};
	txt->pos = pos;
	txt->scale = scale;
	txt->str = cws_string();
	cws_string_build(&txt->str, tx);
	txt->context = context;
	context->texts[context->texts_count++] = txt;
	video_buffer_text_context(context);

	return txt;
}

void cwsRebuildText(cwsTextContext *context, cwsText *text, const char *tx)
{
	cws_string_build(&text->str, tx);
	video_buffer_text_context(context);
}

void cwsDrawTextContext(cwsTextContext *context)
{
	if(context->mesh == NULL)
	{
		return;
	}

    cws_array(f32,data);
	cws_array_init(f32,data,context->texts_count*20);

	mat4 model;
	for(u32 i = 0; i < context->texts_count; ++i)
	{
		vec3 vc = (vec3){.x = (f32)context->texts[i]->pos.x, .y = (f32)context->texts[i]->pos.y, .z = 0.0f};
		model = mat4_translate(mat4_default, vc);
		
		vec3 s = (vec3){.x = context->texts[i]->scale.x, .y = context->texts[i]->scale.y, .z = 1.0f};
		model = mat4_scale(model, s);

		for(i32 j = 0; j < 16; ++j)
		{
			cws_array_push(data,model.m[j]);
		}

		cws_array_push(data,context->texts[i]->bounds.x);
		cws_array_push(data,context->texts[i]->bounds.y);
		cws_array_push(data,context->texts[i]->bounds.z);
		cws_array_push(data,context->texts[i]->bounds.w);
	}

	glBindBuffer(GL_TEXTURE_BUFFER, context->tex_buffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(f32) * data.length, &data.data[0], GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glUseProgram(text_material.shader.id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, context->texture->id);
	glUniform1i(glGetUniformLocation(text_material.shader.id, "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, context->buffer_texture_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, context->tex_buffer);
	glUniform1i(glGetUniformLocation(text_material.shader.id, "instance_data"), 1);

	glUniformMatrix4fv(text_material.shader.mvp_id, 1, GL_FALSE, main_projection_matrix.m);
	glBindVertexArray(context->mesh->vao);
	glDrawElements(GL_TRIANGLES, context->mesh->_icount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	cws_array_free(data);
}

void cwsSetPVMatrices(mat4 projection, mat4 view)
{
	main_projection_matrix = projection;
	main_view_matrix = view;
}

void cwsClear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cwsSwapBuffers()
{
	SDL_GL_SwapWindow(main_window);
}

vec2 cwsScreenSize()
{
	i32 w = 0, h = 0;
	SDL_GetWindowSize(main_window, &w, &h);
	return (vec2){.x = (f32)w, .y = (f32)h};
}

i32 cwsPackRgb(ivec3 rgb)
{
    return rgb.x + ((rgb.y & 255) << 8) + ((rgb.z & 255) << 16);
}

vec3 cwsUnpackRgb(i32 rgb)
{
    f32 a = 1.0f / 255.0f;
    return (vec3){.x = (f32)(rgb&255) * a,
                  .y = (f32)((rgb>>8)&255) * a,
                  .z = (f32)((rgb>>16)&255) * a};
}

void cwsSetPixel(SDL_Surface *s, i32 x, i32 y, u32 c)
{
	int bpp = s->format->BytesPerPixel;
    u32 *p = (u32 *)s->pixels + y * s->pitch + x * bpp;
    (*p) = c;
}

u32 cwsGetPixel(SDL_Surface *s, i32 x, i32 y)
{
	if(x < 0 || x >= s->w || y < 0 || y >= s->h)
	{
		return 0;
	}

    //Convert the pixels to 32 bit 
    u32 *pixels = (u32 *)s->pixels; 

    //Get the requested pixel 
    return pixels[ ( y * s->w ) + x ];
}

void cwsClearColor(vec3 color)
{
    glClearColor(color.x,color.y,color.z,1.0f);
}
