// gx_tev.cpp
// (c) 2005,2009 Gekko Team

#include "..\emu.h"
#include "glew\glew.h"
#include "..\opengl.h"
#include "..\low level\hardware core\hw_gx.h"
#include "gx_tev.h"
#include "..\crc.h"

////////////////////////////////////////////////////////////////////////////////

bool					gx_tev::is_modified;
int						gx_tev::cache_count;
u32						gx_tev::cache_table[MAX_SHADERCACHE_SIZE];
gx_tev::cache_data		gx_tev::cache_program[MAX_SHADERCACHE_SIZE];

int						gx_tev::active_stages;
int						gx_tev::active_texs;
int						gx_tev::active_colors;

u32						gx_tev::combiner[32];
f32						gx_tev::r_cc[4][4];
f32						gx_tev::r_kc[4][4];

GLint					gx_tev::u_cc[4];
GLint					gx_tev::u_kc[4];

const float				epsilon = 1.0f/255.0f;

u8						vsrc[1024];
u8						fsrc[16384];

////////////////////////////////////////////////////////////////////////////////
// TEV FUNCTIONS

u32 gx_tev::get_hash(void)
{
/*
	u8 i;
	u32 hash = bp.genmode._u32 & 0x3BC0F;

	for (i=0; i<(bp.genmode.ntev+1); i++)
	{
		hash = _rotl(hash,3) ^ bp.combiner[i].color._u32;
		hash = _rotl(hash,7) ^ (bp.combiner[i].alpha._u32&0xFFFFFFFC);
	}

	for (i=0; i<(bp.genmode.ntev/2+1); i++)
	{
		hash = _rotl(hash,13) ^ bp.tevorder[i]._u32;
	}

	for (i=0; i<4; i++)
	{
		u8 konst_hash = (u8)(r_kc[i][0] * 16) ^ (u8)(r_kc[i][1] * 16) ^ (u8)(r_kc[i][2] * 16) ^ (u8)(r_kc[i][3] * 16);
		hash = _rotl(hash,3*i) ^ (konst_hash & 0xff);
	}
	hash ^= (bp.cmode1._u32 & 0x100) ^ 0xc0debabe;
	return hash;
*/
	u32 ulCRC = - 1;
	u32 i;

	//generate a hash based off of CRC32 to attempt to avoid collisions
	ulCRC = ulCRC ^ (bp.genmode._u32 & 0x3BC0F);
	ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
			crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
			crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
			crc32_table[0][((ulCRC >> 24))];

	for(i = 0; i < (bp.genmode.ntev+1); i++)
	{
		ulCRC = ulCRC ^ bp.combiner[i].color._u32;
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];

		ulCRC = ulCRC ^ (bp.combiner[i].alpha._u32&0xFFFFFFFC);
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
	}

	for (i=0; i<(bp.genmode.ntev/2+1); i++)
	{
		ulCRC = ulCRC ^ bp.tevorder[i]._u32;
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
	}
/*
	for (i=0; i<4; i++)
	{
		ulCRC = ulCRC ^ *(u32 *)(&r_kc[i][0]);
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];

		ulCRC = ulCRC ^ *(u32 *)(&r_kc[i][1]);
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];

		ulCRC = ulCRC ^ *(u32 *)(&r_kc[i][2]);
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];

		ulCRC = ulCRC ^ *(u32 *)(&r_kc[i][3]);
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
				crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
				crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
	}
*/

	return ulCRC;
}

void gx_tev::upload_color(u8 _addr, u32 _value)
{
	if(_addr & 1) // green/blue
	{
		if(!(_value >> 23)) // color
		{
			// unpack
			r_cc[(_addr - 0xe1) / 2][1] = ((_value >> 12) & 0xff) / 255.0f;
			r_cc[(_addr - 0xe1) / 2][2] = ((_value >> 0) & 0xff) / 255.0f;


		}else{ // konstant
			// unpack
			r_kc[(_addr - 0xe1) / 2][1] = ((_value >> 12) & 0xff) / 255.0f;
			r_kc[(_addr - 0xe1) / 2][2] = ((_value >> 0) & 0xff) / 255.0f;

			set_modifed();
		}
	}else{ // red/alpha
		if(!(_value >> 23)) // color
		{
			// unpack
			r_cc[(_addr - 0xe0) / 2][3] = ((_value >> 12) & 0xff) / 255.0f;
			r_cc[(_addr - 0xe0) / 2][0] = ((_value >> 0) & 0xff) / 255.0f;
		}else{ // konstant
			// unpack
			r_kc[(_addr - 0xe0) / 2][3] = ((_value >> 12) & 0xff) / 255.0f;
			r_kc[(_addr - 0xe0) / 2][0] = ((_value >> 0) & 0xff) / 255.0f;

			set_modifed();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// SHADER FUNCTIONS

void gx_tev::generate_vertex_program(void)
{
	char	*vs;
	char	vertex_program1[] = "void main() {\n";
	char	vertex_program2a[] = "gl_FrontColor = gl_Color;\n";
	char	vertex_program2b[] = "gl_FrontColor = vec4(";
	char	vertex_program3[] = "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
								"gl_TexCoord[1] = gl_TextureMatrix[0] * gl_MultiTexCoord1;\n"
								"gl_TexCoord[2] = gl_TextureMatrix[0] * gl_MultiTexCoord2;\n"
								"gl_TexCoord[3] = gl_TextureMatrix[0] * gl_MultiTexCoord3;\n"
								"gl_TexCoord[4] = gl_TextureMatrix[0] * gl_MultiTexCoord4;\n"
								"gl_TexCoord[5] = gl_TextureMatrix[0] * gl_MultiTexCoord5;\n"
								"gl_TexCoord[6] = gl_TextureMatrix[0] * gl_MultiTexCoord6;\n"
								"gl_TexCoord[7] = gl_TextureMatrix[0] * gl_MultiTexCoord7;\n"
								"gl_Position = ftransform();\n"
								"}";
	u32		offset;
	
	//vs = (char*)malloc(sizeof(vertex_program1) + sizeof(vertex_program3) + sizeof(vertex_program2a) + 60);

	vs = (char *)&vsrc[0];

	memcpy(&vs[0], vertex_program1, sizeof(vertex_program1) - 1);
	offset = sizeof(vertex_program1) - 1;

	if(xf.colorcontrol[0].materialsrc)
	{
		memcpy(&vs[offset], vertex_program2a, sizeof(vertex_program2a) - 1);
		offset += sizeof(vertex_program2a) - 1;
	}
	else
	{
		memcpy(&vs[offset], vertex_program2b, sizeof(vertex_program2b) - 1);
		offset += sizeof(vertex_program2b) - 1;
		offset += sprintf(&vs[offset], "%f, %f, %f, 1.0);\n", xf.material[0].r / 255.0f, xf.material[0].g / 255.0f, xf.material[0].b / 255.0f);
	}

	memcpy(&vs[offset], vertex_program3, sizeof(vertex_program3) - 1);
	offset += sizeof(vertex_program3) - 1;

	//insert the null at the end
	vs[offset] = 0;

	return; // vs;
}

void gx_tev::generate_fragment_program(void)
{
	char *fs = (char *)&fsrc[0]; // = (char*)malloc(MAX_TEV_PROGRAM);
	u32	 offset;

//	fs = fsrc;
	fs[0] = 0;

	//memset(fs,0,MAX_TEV_PROGRAM);
//	s_comment(fs, "begin gekko gfx fragment shader - shizzy");

	// define dynamic variables
	char	fsvars[] = "uniform sampler2D texture0;"
						"uniform sampler2D texture1;"
						"uniform sampler2D texture2;"
						"uniform sampler2D texture3;"
						"uniform sampler2D texture4;"
						"uniform sampler2D texture5;"
						"uniform sampler2D texture6;"
						"uniform sampler2D texture7;"
						"uniform vec4 input0;"
						"uniform vec4 input1;"
						"uniform vec4 input2;"
						"uniform vec4 input3;"
						"void main(){"
						"const vec4	cc_zero = vec4(0.0, 0.0, 0.0, 0.0);"
						"const vec4	cc_half = vec4(0.5, 0.5, 0.5, 0.5);"
						"const vec4	cc_one = vec4(1.0, 1.0, 1.0, 1.0);"
						"vec4			cc_texc;"
						"vec4			cc_rasc;"
						"vec4			cc_cprev = input0;"
						"vec4			cc_c0 = input1;"
						"vec4			cc_c1 = input2;"
						"vec4			cc_c2 = input3;"
						"vec4			konst;"
						"vec4			dest;"
						"vec4			ta;"
						"vec4			tb;"
						"vec4			tc;"
						"bvec3		bv1;"
						"bvec3		bv2;";


	////////////////////////////////////////////////////////////
	
	//s_out(fs, "void main(){"); // BEGIN MAIN

	int i;

	fs[0] = 0;
	offset = 0;

	memcpy(&fs[offset], fsvars, sizeof(fsvars));
//	fs[sizeof(fsvars)] = 0;

	// define constant variables
	offset += sizeof(fsvars) - 1;
	offset += sprintf(&fs[offset], "const vec4	kc_c0 = vec4(%f, %f, %f, %f);", r_kc[0][0], r_kc[0][1], r_kc[0][2], r_kc[0][3]);
	offset += sprintf(&fs[offset], "const vec4	kc_c1 = vec4(%f, %f, %f, %f);", r_kc[1][0], r_kc[1][1], r_kc[1][2], r_kc[1][3]);
	offset += sprintf(&fs[offset], "const vec4	kc_c2 = vec4(%f, %f, %f, %f);", r_kc[2][0], r_kc[2][1], r_kc[2][2], r_kc[2][3]);
	offset += sprintf(&fs[offset], "const vec4	kc_c3 = vec4(%f, %f, %f, %f);", r_kc[3][0], r_kc[3][1], r_kc[3][2], r_kc[3][3]);

	for(i = 0; i < active_stages; i++)
	{
//		s_comment(fs, "stage %d", i);

		// set the konstant color

		offset += sprintf(&fs[offset], "konst.rgb = %s;", tev_konst_colorop[bp.ksel[i >> 1].getkc(i)]);
		offset += sprintf(&fs[offset], "konst.a = %s;", tev_konst_alphaop[bp.ksel[i >> 1].getka(i)]);

		// set the raster color

		switch(bp.tevorder[i >> 1].get_colorchan(i))
		{
		case 0: offset += sprintf(&fs[offset], "cc_rasc = gl_Color;"); break;
		case 1: offset += sprintf(&fs[offset], "cc_rasc = gl_SecondaryColor;"); break;
		case 2: offset += sprintf(&fs[offset], "cc_rasc = gl_Color;"); break;
		case 3: offset += sprintf(&fs[offset], "cc_rasc = gl_SecondaryColor;"); break;
		case 4: offset += sprintf(&fs[offset], "cc_rasc = gl_Color;"); break;
		case 5: offset += sprintf(&fs[offset], "cc_rasc = gl_SecondaryColor;"); break;
		case 6: offset += sprintf(&fs[offset], "cc_rasc = vec4(0.0, 0.0, 0.0, 0.0);"); break;
		// case 7 todo
		// case 8 ..
		default: offset += sprintf(&fs[offset], "cc_rasc = vec4(0.0, 1.0, 0.0, 1.0);"); break;
		}

		// set the texture color

		if(bp.tevorder[i >> 1].get_enable(i))
			offset += sprintf(&fs[offset], "cc_texc = texture2D(texture%d, gl_TexCoord[%d].st);", bp.tevorder[i >> 1].get_texmap(i), bp.tevorder[i >> 1].get_texcoord(i));
		else
			offset += sprintf(&fs[offset], "cc_texc = cc_one;");

		// color op

		offset += sprintf(&fs[offset], "dest.rgb = %s(%s %s (mix(%s, %s, %s) %s*cc_one.rgb));", 
			tev_scale[bp.combiner[i].color.shift],
			tev_colorop_input[bp.combiner[i].color.seld],
			tev_sub[bp.combiner[i].color.sub], 
			tev_colorop_input[bp.combiner[i].color.sela], 
			tev_colorop_input[bp.combiner[i].color.selb], 
			tev_colorop_input[bp.combiner[i].color.selc], 
			tev_bias[bp.combiner[i].color.bias]
		);

		if(bp.combiner[i].color.clamp)
			offset += sprintf(&fs[offset], "dest.rgb = clamp(dest.rgb, cc_zero.rgb, cc_one.rgb);");

		// alpha op

		offset += sprintf(&fs[offset], "dest.a = %s(%s %s (mix(%s, %s, %s) %s));", 
			tev_scale[bp.combiner[i].alpha.shift],
			tev_alphaop_input[bp.combiner[i].alpha.seld],
			tev_sub[bp.combiner[i].alpha.sub],
			tev_alphaop_input[bp.combiner[i].alpha.sela], 
			tev_alphaop_input[bp.combiner[i].alpha.selb], 
			tev_alphaop_input[bp.combiner[i].alpha.selc],  
			tev_bias[bp.combiner[i].alpha.bias]
		); 

		if(bp.combiner[i].alpha.clamp)
			offset += sprintf(&fs[offset], "dest.a = clamp(dest.a, 0.0, 1.0);");

		if(i + 1 < active_stages)
		{
			offset += sprintf(&fs[offset], "%s = dest.rgb;", tev_colorop_output[bp.combiner[i].color.dest]);
			offset += sprintf(&fs[offset], "%s = dest.a;", tev_alphaop_output[bp.combiner[i].alpha.dest]);
		}
	}

	if(bp.cmode1.enable)
	{
		offset += sprintf(&fs[offset], "gl_FragColor.rgb = dest.rgb;");
		offset += sprintf(&fs[offset], "gl_FragColor.a = %f;", bp.cmode1.getalpha());
	}else{
		offset += sprintf(&fs[offset], "gl_FragColor.rgb = dest.rgb;");

		if(gx_cfg.force_alpha)
			offset += sprintf(&fs[offset], "gl_FragColor.a = dest.a / 2;");	// 50% alpha mode
		else
			offset += sprintf(&fs[offset], "gl_FragColor.a = dest.a;");
	}

	//offset += sprintf(&fs[offset], "gl_FragColor.rgb = cc_one.rgb;");
	//offset += sprintf(&fs[offset], "gl_FragColor.a = cc_one.a;");

	offset += sprintf(&fs[offset], "}"); // END MAIN

	////////////////////////////////////////////////////////////
	
//	s_comment(fs, "end shader");

	return; // fs;
}

void gx_tev::generate_shader(void)
{
	GLhandleARB fp, vp;
	u32 hash = get_hash();
	u32 entry = check_cache_entry(hash);
	u32 i;

	// if our shader has not already been made...
	if(entry > MAX_SHADERCACHE_SIZE)
	{
		// check if shader cache has overflown
		if(cache_count > MAX_SHADERCACHE_SIZE)
		{
			for(i = 0; i < cache_count; i++)
				glDeleteObjectARB(cache_program[i].program);

			cache_count = 0;
			memset(&cache_program, 0, MAX_SHADERCACHE_SIZE * sizeof(cache_data));
			printf("Out of cache space!\n");
		}

		// add hash to the cache
		cache_table[cache_count] = hash;

		// generate vertex/fragment shaders
		generate_vertex_program();
		generate_fragment_program();
		const char *vsrc2 = (char *)&vsrc[0];
		const char *fsrc2 = (char *)&fsrc[0];

		// create vertex/fragment object
		vp = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		fp = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

		// load vertex/fragment source
		glShaderSourceARB(vp, 1, &vsrc2, NULL);
		glShaderSourceARB(fp, 1, &fsrc2, NULL);

		// compile vertex/fragment source
		glCompileShaderARB(vp);
		glCompileShaderARB(fp);

		// create shader program
		cache_program[cache_count].program = glCreateProgramObjectARB();

		// attach vertex/fragments to program
		glAttachObjectARB(cache_program[cache_count].program,vp);
		glAttachObjectARB(cache_program[cache_count].program,fp);
		
		// link program
		glLinkProgramARB(cache_program[cache_count].program);

		// update shader count
		entry = cache_count++;

		// use shader
		glUseProgramObjectARB(cache_program[entry].program);

		// get location of uniform shader variables
		cache_program[entry].u_cc[0] = glGetUniformLocationARB(cache_program[entry].program,"input0");
		cache_program[entry].u_cc[1] = glGetUniformLocationARB(cache_program[entry].program,"input1");
		cache_program[entry].u_cc[2] = glGetUniformLocationARB(cache_program[entry].program,"input2");
		cache_program[entry].u_cc[3] = glGetUniformLocationARB(cache_program[entry].program,"input3");

		cache_program[entry].u_t[0] = glGetUniformLocationARB(cache_program[entry].program, "texture0");
		cache_program[entry].u_t[1] = glGetUniformLocationARB(cache_program[entry].program, "texture1");
		cache_program[entry].u_t[2] = glGetUniformLocationARB(cache_program[entry].program, "texture2");
		cache_program[entry].u_t[3] = glGetUniformLocationARB(cache_program[entry].program, "texture3");
		cache_program[entry].u_t[4] = glGetUniformLocationARB(cache_program[entry].program, "texture4");
		cache_program[entry].u_t[5] = glGetUniformLocationARB(cache_program[entry].program, "texture5");
		cache_program[entry].u_t[6] = glGetUniformLocationARB(cache_program[entry].program, "texture6");
		cache_program[entry].u_t[7] = glGetUniformLocationARB(cache_program[entry].program, "texture7");
	}

	// use shader
	glUseProgramObjectARB(cache_program[entry].program);

	// send as uniform data
	if(cache_program[entry].u_cc[0] != -1)
		glUniform4fvARB(cache_program[entry].u_cc[0], 1, r_cc[0]);
	if(cache_program[entry].u_cc[1] != -1)
		glUniform4fvARB(cache_program[entry].u_cc[1], 1, r_cc[1]);
	if(cache_program[entry].u_cc[2] != -1)
		glUniform4fvARB(cache_program[entry].u_cc[2], 1, r_cc[2]);
	if(cache_program[entry].u_cc[3] != -1)
		glUniform4fvARB(cache_program[entry].u_cc[3], 1, r_cc[3]);


	for(i = 0; i < 8; i++)
	{
		if(cache_program[entry].u_t[i] != -1)
		{
			glUniform1iARB(cache_program[entry].u_t[i], i);
		}
	}

	// reset flag
	is_modified = false;
}

////////////////////////////////////////////////////////////////////////////////
// CACHE FUNCTIONS

// sets the tev has been modified flag
void gx_tev::set_modifed(void)
{
	is_modified = true;
}

// checks if a cache entry has already been made of current tev
int gx_tev::check_cache_entry(u32 _hash)
{
	for(int i = 0; i < cache_count; i++)
	{
		if(cache_table[i] == _hash) return i;
	}

	return (MAX_SHADERCACHE_SIZE + 1);
}

// adds last generated shader info to cache (shader itself already pointer)
void gx_tev::add_to_cache(void)
{
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void gx_tev::initialize(void)
{
}

void gx_tev::destroy(void)
{
	is_modified = true;
	cache_count = 0;
	memset(&cache_table, 0, MAX_SHADERCACHE_SIZE * 4);
	memset(&cache_program, 0, MAX_SHADERCACHE_SIZE * sizeof(GLhandleARB));
}

////////////////////////////////////////////////////////////////////////////////
// EOF
