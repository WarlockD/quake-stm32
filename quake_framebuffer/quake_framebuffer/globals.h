#ifndef _QUAKE_GLOBALS_H_
#define _QUAKE_GLOBALS_H_

// used cproto to cheat and get these all created

// zone.c
typedef struct memblock_s
{
	int		size;           // including the header and possibly tiny fragments
	int     tag;            // a tag of 0 is a free block
	int     id;        		// should be ZONEID
	struct memblock_s       *next, *prev;
	int		pad;			// pad to 64 bit boundary
} memblock_t;

typedef struct
{
	int		size;		// total bytes malloced, including header
	memblock_t	blocklist;		// start / end cap for linked list
	memblock_t	*rover;
} memzone_t;


typedef struct 
{

	// All private and static vars
	/* cd_null.c */
	/* chase.c */
	/* cl_demo.c */
	/* cl_input.c */
	/* cl_main.c */
	/* cl_parse.c */
	/* cl_tent.c */
	/* cmd.c */
	 int cmd_argc;
	 char *cmd_argv[MAX_ARGS];
	 char *cmd_null_string;
	 char *cmd_args;
	 cmd_function_t *cmd_functions;
	/* common.c */
	 char *largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
	 char *argvdummy;
	 char *safeargvs[NUM_SAFE_ARGVS];
	/* conproc.c */
	/* console.c */
	/* crc.c */
	 unsigned short crctable[256];
	/* cvar.c */
	/* d_edge.c */
	 int miplevel;
	/* d_fill.c */
	/* d_init.c */
	 float basemip[NUM_MIPS - 1];
	/* d_modech.c */
	/* d_part.c */
	/* d_polyse.c */
	 int ystart;
	 adivtab_t adivtab[32 * 32];
	/* d_scan.c */
	/* d_sky.c */
	/* d_sprite.c */
	 int sprite_height;
	 int minindex;
	 int maxindex;
	 sspan_t *sprite_spans;
	/* d_surf.c */
	/* d_vars.c */
	/* d_zpoint.c */
	/* draw.c */
	 rectdesc_t r_rectdesc;
	/* gl_draw.c */
	/* gl_mesh.c */
	/* gl_model.c */
	/* gl_refrag.c */
	/* gl_rlight.c */
	/* gl_rmain.c */
	/* gl_rmisc.c */
	/* gl_rsurf.c */
	/* gl_screen.c */
	 void SCR_CalcRefdef(void);
	/* gl_test.c */
	/* gl_vidlinux.c */
	 unsigned char scantokey[128];
	 float vid_gamma;
	 int resolutions[NUM_RESOLUTIONS][3];

	/* gl_vidlinuxglx.c */
	 int scrnum;
	 qboolean mouse_avail;
	 qboolean mouse_active;
	 int mx;
	 int my;
	 int old_mouse_x;
	 int old_mouse_y;
	 cvar_t in_mouse;
	 cvar_t in_dgamouse;
	 cvar_t m_filter;
	 int win_x;
	 int win_y;
	 int scr_width;
	 int scr_height;
	 int default_dotclock_vidmode;
	 int num_vidmodes;
	 qboolean vidmode_active;
	 float vid_gamma;

	/* gl_vidnt.c */
	 vmode_t modelist[MAX_MODE_LIST];
	 int nummodes;
	 vmode_t *pcurrentmode;
	 vmode_t badmode;
	 qboolean vid_initialized;
	 qboolean windowed;
	 qboolean leavecurrentmode;
	 qboolean vid_canalttab;
	 qboolean vid_wassuspended;
	 int windowed_mouse;
	 int windowed_default;
	 qboolean fullsbardraw;
	 float vid_gamma;

	 int vid_line;
	 int vid_wmodes;
	 modedesc_t modedescs[MAX_MODEDESCS];
	/* gl_warp.c */
	/* host.c */
	/* host_cmd.c */
	/* in_null.c */
	 const byte scantokey[128];
	/* keys.c */
	/* mathlib.c */
	/* menu.c */
	 int ISA_uarts[];
	 int ISA_IRQs[];
	/* model.c */
	/* net_loop.c */
	 int IntAlign(int value);
	/* net_main.c */
	 qboolean listening;
	 double slistStartTime;
	 int slistLastShown;

	 PollProcedure *pollProcedureList;
	/* net_none.c */
	/* net_vcr.c */
	 struct {} next;
	/* nonintel.c */
	/* pr_cmds.c */
	/* pr_edict.c */
	 gefv_cache gefvCache[GEFV_CACHESIZE];
	/* pr_exec.c */
	/* r_aclip.c */
	 finalvert_t fv[2][8];
	 auxvert_t av[8];
	/* r_alias.c */
	 float ziscale;
	 model_t *pmodel;
	 vec3_t alias_forward;
	 vec3_t alias_right;
	 vec3_t alias_up;
	 maliasskindesc_t *pskindesc;
	 aedge_t aedges[12];
	/* r_bsp.c */
	 mvertex_t *pbverts;
	 bedge_t *pbedges;
	 int numbverts;
	 int numbedges;
	 mvertex_t *pfrontenter;
	 mvertex_t *pfrontexit;
	 qboolean makeclippededge;
	/* r_draw.c */
	 qboolean makeleftedge;
	 qboolean makerightedge;
	/* r_edge.c */
	 void(*pdrawfunc)(void);
	/* r_efrag.c */
	/* r_light.c */
	/* r_main.c */
	/* r_misc.c */
	/* r_part.c */
	/* r_sky.c */
	/* r_sprite.c */
	 int clip_current;
	 vec5_t clip_verts[2][MAXWORKINGVERTS];
	 int sprite_width;
	 int sprite_height;
	/* r_surf.c */
	 void(*surfmiptable[4])(void);
	/* r_vars.c */
	/* sbar.c */
	/* screen.c */
	/* snd_mem.c */
	/* snd_mix.c */
	/* snd_next.c */
	/* snd_null.c */
	/* sv_main.c */
	/* sv_move.c */
	/* sv_phys.c */
	 vec3_t vec_origin;
	/* sv_user.c */
	 vec3_t forward;
	 vec3_t right;
	 vec3_t up;

	/* view.c */
	/* wad.c */
	/* world.c */
	hull_t box_hull;
	dclipnode_t box_clipnodes[6];
	mplane_t box_planes[6];
	areanode_t sv_areanodes[AREA_NODES];
	int sv_numareanodes;
	/* zone.c */

	// all public or externs
	/* cd_null.c */
	/* chase.c */
	cvar_t chase_back;
	cvar_t chase_up;
	cvar_t chase_right;
	cvar_t chase_active;
	vec3_t chase_pos;
	vec3_t chase_angles;
	vec3_t chase_dest;
	vec3_t chase_dest_angles;
	/* cl_demo.c */
	/* cl_input.c */
	kbutton_t in_mlook;
	kbutton_t in_klook;
	kbutton_t in_left;
	kbutton_t in_right;
	kbutton_t in_forward;
	kbutton_t in_back;
	kbutton_t in_lookup;
	kbutton_t in_lookdown;
	kbutton_t in_moveleft;
	kbutton_t in_moveright;
	kbutton_t in_strafe;
	kbutton_t in_speed;
	kbutton_t in_use;
	kbutton_t in_jump;
	kbutton_t in_attack;
	kbutton_t in_up;
	kbutton_t in_down;
	int in_impulse;
	cvar_t cl_upspeed;
	cvar_t cl_forwardspeed;
	cvar_t cl_backspeed;
	cvar_t cl_sidespeed;
	cvar_t cl_movespeedkey;
	cvar_t cl_yawspeed;
	cvar_t cl_pitchspeed;
	cvar_t cl_anglespeedkey;
	/* cl_main.c */
	cvar_t cl_name;
	cvar_t cl_color;
	cvar_t cl_shownet;
	cvar_t cl_nolerp;
	cvar_t lookspring;
	cvar_t lookstrafe;
	cvar_t sensitivity;
	cvar_t m_pitch;
	cvar_t m_yaw;
	cvar_t m_forward;
	cvar_t m_side;
	client_static_t cls;
	client_state_t cl;
	efrag_t cl_efrags[MAX_EFRAGS];
	entity_t cl_entities[MAX_EDICTS];
	entity_t cl_static_entities[MAX_STATIC_ENTITIES];
	lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
	dlight_t cl_dlights[MAX_DLIGHTS];
	int cl_numvisedicts;
	entity_t *cl_visedicts[MAX_VISEDICTS];
	/* cl_parse.c */
	char *svc_strings[];
	int bitcounts[16];
	/* cl_tent.c */
	int num_temp_entities;
	entity_t cl_temp_entities[MAX_TEMP_ENTITIES];
	beam_t cl_beams[MAX_BEAMS];
	sfx_t *cl_sfx_wizhit;
	sfx_t *cl_sfx_knighthit;
	sfx_t *cl_sfx_tink1;
	sfx_t *cl_sfx_ric1;
	sfx_t *cl_sfx_ric2;
	sfx_t *cl_sfx_ric3;
	sfx_t *cl_sfx_r_exp3;
	sfx_t *cl_sfx_imp;
	sfx_t *cl_sfx_rail;
	/* cmd.c */
	cmdalias_t *cmd_alias;
	int trashtest;
	int *trashspot;
	qboolean cmd_wait;
	sizebuf_t cmd_text;
	cmd_source_t cmd_source;
	/* common.c */
	cvar_t registered;
	cvar_t cmdline;
	qboolean com_modified;
	qboolean proghack;
	int static_registered;
	qboolean msg_suppress_1;
	char com_token[1024];
	int com_argc;
	char **com_argv;
	char com_cmdline[CMDLINE_LENGTH];
	qboolean standard_quake;
	qboolean rogue;
	qboolean hipnotic;
	unsigned short pop[];
	qboolean bigendien;
	short(*BigShort)(short l);
	short(*LittleShort)(short l);
	int(*BigLong)(int l);
	int(*LittleLong)(int l);
	float(*BigFloat)(float l);
	float(*LittleFloat)(float l);
	int msg_readcount;
	qboolean msg_badread;
	int com_filesize;
	char com_cachedir[MAX_OSPATH];
	char com_gamedir[MAX_OSPATH];
	searchpath_t *com_searchpaths;
	cache_user_t *loadcache;
	byte *loadbuf;
	int loadsize;
	/* conproc.c */
	/* console.c */
	int con_linewidth;
	float con_cursorspeed;
	qboolean con_forcedup;
	int con_totallines;
	int con_backscroll;
	int con_current;
	int con_x;
	char *con_text;
	cvar_t con_notifytime;
	float con_times[NUM_CON_TIMES];
	int con_vislines;
	qboolean con_debuglog;
	qboolean con_initialized;
	int con_notifylines;
	/* crc.c */
	unsigned /* cvar.c */
		cvar_t *cvar_vars;
	char *cvar_null_string;
	/* d_edge.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	float scale_for_mip;
	int screenwidth;
	int ubasestep;
	int errorterm;
	int erroradjustup;
	int erroradjustdown;
	int vstartscan;
	vec3_t transformed_modelorg;
	/* d_fill.c */
	/* d_init.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	cvar_t d_subdiv16;
	cvar_t d_mipcap;
	cvar_t d_mipscale;
	surfcache_t *d_initial_rover;
	qboolean d_roverwrapped;
	int d_minmip;
	float d_scalemip[NUM_MIPS - 1];
	void(*d_drawspans)(espan_t *pspan);
	/* d_modech.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	int d_vrectx;
	int d_vrecty;
	int d_vrectright_particle;
	int d_vrectbottom_particle;
	int d_y_aspect_shift;
	int d_pix_min;
	int d_pix_max;
	int d_pix_shift;
	int d_scantable[MAXHEIGHT];
	short *zspantable[MAXHEIGHT];
	/* d_part.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	/* d_polyse.c */
	int r_p0[6];
	int r_p1[6];
	int r_p2[6];
	byte *d_pcolormap;
	int d_aflatcolor;
	int d_xdenom;
	edgetable *pedgetable;
	edgetable edgetables[12];
	int a_sstepxfrac;
	int a_tstepxfrac;
	int r_lstepx;
	int a_ststepxwhole;
	int r_sstepx;
	int r_tstepx;
	int r_lstepy;
	int r_sstepy;
	int r_tstepy;
	int r_zistepx;
	int r_zistepy;
	int d_aspancount;
	int d_countextrastep;
	spanpackage_t *a_spans;
	spanpackage_t *d_pedgespanpackage;
	byte *d_pdest;
	byte *d_ptex;
	short *d_pz;
	int d_sfrac;
	int d_tfrac;
	int d_light;
	int d_zi;
	int d_ptexextrastep;
	int d_sfracextrastep;
	int d_tfracextrastep;
	int d_lightextrastep;
	int d_pdestextrastep;
	int d_lightbasestep;
	int d_pdestbasestep;
	int d_ptexbasestep;
	int d_sfracbasestep;
	int d_tfracbasestep;
	int d_ziextrastep;
	int d_zibasestep;
	int d_pzextrastep;
	int d_pzbasestep;
	byte *skintable[MAX_LBM_HEIGHT];
	int skinwidth;
	byte *skinstart;
	byte gelmap[256];
	/* d_scan.c */
	unsigned char *r_turb_pbase;
	unsigned char *r_turb_pdest;
	fixed16_t r_turb_s;
	fixed16_t r_turb_t;
	fixed16_t r_turb_sstep;
	fixed16_t r_turb_tstep;
	int *r_turb_turb;
	int r_turb_spancount;
	/* d_sky.c */
	/* d_sprite.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	/* d_surf.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	float surfscale;
	qboolean r_cache_thrash;
	int sc_size;
	surfcache_t *sc_rover;
	surfcache_t *sc_base;
	/* d_vars.c */
	float d_sdivzstepu;
	float d_tdivzstepu;
	float d_zistepu;
	float d_sdivzstepv;
	float d_tdivzstepv;
	float d_zistepv;
	float d_sdivzorigin;
	float d_tdivzorigin;
	float d_ziorigin;
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	pixel_t *cacheblock;
	int cachewidth;
	pixel_t *d_viewbuffer;
	short *d_pzbuffer;
	unsigned int d_zrowbytes;
	unsigned int d_zwidth;
	/* d_zpoint.c */
	fixed16_t sadjust;
	fixed16_t tadjust;
	fixed16_t bbextents;
	fixed16_t bbextentt;
	void(*prealspandrawer)(void);
	/* draw.c */
	byte *draw_chars;
	qpic_t *draw_disc;
	qpic_t *draw_backtile;
	cachepic_t menu_cachepics[MAX_CACHED_PICS];
	int menu_numcachepics;
	/* gl_draw.c */
	cvar_t gl_nobind;
	cvar_t gl_max_size;
	cvar_t gl_picmip;
	byte *draw_chars;
	qpic_t *draw_disc;
	qpic_t *draw_backtile;
	int translate_texture;
	int char_texture;
	byte conback_buffer[sizeof(qpic_t) + sizeof(glpic_t)];
	qpic_t *conback;
	int gl_lightmap_format;
	int gl_solid_format;
	int gl_alpha_format;
	int gl_filter_min;
	int gl_filter_max;
	int texels;
	gltexture_t gltextures[MAX_GLTEXTURES];
	int numgltextures;
	int scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
	byte scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT * 4];
	qboolean scrap_dirty;
	int scrap_texnum;
	int scrap_uploads;
	cachepic_t menu_cachepics[MAX_CACHED_PICS];
	int menu_numcachepics;
	byte menuplyr_pixels[4096];
	int pic_texels;
	int pic_count;
	glmode_t modes[];
	/* gl_mesh.c */
	model_t *aliasmodel;
	aliashdr_t *paliashdr;
	qboolean used[8192];
	int commands[8192];
	int numcommands;
	int vertexorder[8192];
	int numorder;
	int allverts;
	int alltris;
	int stripverts[128];
	int striptris[128];
	int stripcount;
	/* gl_model.c */
	model_t *loadmodel;
	char loadname[32];
	byte mod_novis[MAX_MAP_LEAFS / 8];
	model_t mod_known[MAX_MOD_KNOWN];
	int mod_numknown;
	cvar_t gl_subdivide_size;
	byte *mod_base;
	aliashdr_t *pheader;
	stvert_t stverts[MAXALIASVERTS];
	mtriangle_t triangles[MAXALIASTRIS];
	trivertx_t *poseverts[MAXALIASFRAMES];
	int posenum;
	byte **player_8bit_texels_tbl;
	byte *player_8bit_texels;
	/* gl_refrag.c */
	mnode_t *r_pefragtopnode;
	efrag_t **lastlink;
	vec3_t r_emins;
	vec3_t r_emaxs;
	entity_t *r_addent;
	/* gl_rlight.c */
	int r_dlightframecount;
	mplane_t *lightplane;
	vec3_t lightspot;
	/* gl_rmain.c */
	entity_t r_worldentity;
	qboolean r_cache_thrash;
	vec3_t modelorg;
	vec3_t r_entorigin;
	entity_t *currententity;
	int r_visframecount;
	int r_framecount;
	mplane_t frustum[4];
	int c_brush_polys;
	int c_alias_polys;
	qboolean envmap;
	int currenttexture;
	int cnttextures[2];
	int particletexture;
	int playertextures;
	int mirrortexturenum;
	qboolean mirror;
	mplane_t *mirror_plane;
	vec3_t vup;
	vec3_t vpn;
	vec3_t vright;
	vec3_t r_origin;
	float r_world_matrix[16];
	float r_base_world_matrix[16];
	refdef_t r_refdef;
	mleaf_t *r_viewleaf;
	mleaf_t *r_oldviewleaf;
	texture_t *r_notexture_mip;
	int d_lightstylevalue[256];
	cvar_t r_norefresh;
	cvar_t r_drawentities;
	cvar_t r_drawviewmodel;
	cvar_t r_speeds;
	cvar_t r_fullbright;
	cvar_t r_lightmap;
	cvar_t r_shadows;
	cvar_t r_mirroralpha;
	cvar_t r_wateralpha;
	cvar_t r_dynamic;
	cvar_t r_novis;
	cvar_t gl_finish;
	cvar_t gl_clear;
	cvar_t gl_cull;
	cvar_t gl_texsort;
	cvar_t gl_smoothmodels;
	cvar_t gl_affinemodels;
	cvar_t gl_polyblend;
	cvar_t gl_flashblend;
	cvar_t gl_playermip;
	cvar_t gl_nocolors;
	cvar_t gl_keeptjunctions;
	cvar_t gl_reporttjunctions;
	cvar_t gl_doubleeyes;
	float r_avertexnormals[NUMVERTEXNORMALS][3];
	vec3_t shadevector;
	float shadelight;
	float ambientlight;
	float r_avertexnormal_dots[SHADEDOT_QUANT][256];
	float *shadedots;
	int lastposenum;
	/* gl_rmisc.c */
	byte dottexture[8][8];
	/* gl_rsurf.c */
	int skytexturenum;
	int lightmap_bytes;
	int lightmap_textures;
	unsigned blocklights[18 * 18];
	int active_lightmaps;
	glpoly_t *lightmap_polys[MAX_LIGHTMAPS];
	qboolean lightmap_modified[MAX_LIGHTMAPS];
	glRect_t lightmap_rectchange[MAX_LIGHTMAPS];
	int allocated[MAX_LIGHTMAPS][BLOCK_WIDTH];
	byte lightmaps[4 * MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];
	msurface_t *skychain;
	msurface_t *waterchain;
	qboolean mtexenabled;
	mvertex_t *r_pcurrentvertbase;
	model_t *currentmodel;
	int nColinElim;
	/* gl_screen.c */
	int glx;
	int gly;
	int glwidth;
	int glheight;
	int scr_copytop;
	int scr_copyeverything;
	float scr_con_current;
	float scr_conlines;
	float oldscreensize;
	float oldfov;
	cvar_t scr_viewsize;
	cvar_t scr_fov;
	cvar_t scr_conspeed;
	cvar_t scr_centertime;
	cvar_t scr_showram;
	cvar_t scr_showturtle;
	cvar_t scr_showpause;
	cvar_t scr_printspeed;
	cvar_t gl_triplebuffer;
	qboolean scr_initialized;
	qpic_t *scr_ram;
	qpic_t *scr_net;
	qpic_t *scr_turtle;
	int scr_fullupdate;
	int clearconsole;
	int clearnotify;
	int sb_lines;
	viddef_t vid;
	vrect_t scr_vrect;
	qboolean scr_disabled_for_loading;
	qboolean scr_drawloading;
	float scr_disabled_time;
	qboolean block_drawing;
	char scr_centerstring[1024];
	float scr_centertime_start;
	float scr_centertime_off;
	int scr_center_lines;
	int scr_erase_lines;
	int scr_erase_center;
	char *scr_notifystring;
	qboolean scr_drawdialog;
	/* gl_test.c */
	puff_t puffs[MAX_PUFFS];
	plane_t junk;
	/* gl_vidlinux.c */
	unsigned short d_8to16table[256];
	unsigned d_8to24table[256];
	unsigned char d_15to8table[65536];
	int num_shades;
	struct {} mice[];
	int num_mice;
	int d_con_indirect;
	int svgalib_inited;
	int UseMouse;
	int UseKeyboard;
	int mouserate;
	cvar_t vid_mode;
	cvar_t vid_redrawfull;
	cvar_t vid_waitforrefresh;
	char *framebuffer_ptr;
	cvar_t mouse_button_commands[3];
	int mouse_buttons;
	int mouse_buttonstate;
	int mouse_oldbuttonstate;
	float mouse_x;
	float mouse_y;
	float old_mouse_x;
	float old_mouse_y;
	int mx;
	int my;
	cvar_t m_filter;
	int scr_width;
	int scr_height;
	int texture_mode;
	int texture_extension_number;
	float gldepthmin;
	float gldepthmax;
	cvar_t gl_ztrick;
	const char *gl_vendor;
	const char *gl_renderer;
	const char *gl_version;
	const char *gl_extensions;
	void(*qglColorTableEXT)(int, int, int, int, int, const void *);
	qboolean is8bit;
	qboolean isPermedia;
	qboolean gl_mtexable;
	/* gl_vidlinuxglx.c */
	unsigned short d_8to16table[256];
	unsigned d_8to24table[256];
	unsigned char d_15to8table[65536];
	cvar_t vid_mode;
	qboolean dgamouse;
	qboolean vidmode_ext;
	int texture_mode;
	int texture_extension_number;
	float gldepthmin;
	float gldepthmax;
	cvar_t gl_ztrick;
	const char *gl_vendor;
	const char *gl_renderer;
	const char *gl_version;
	const char *gl_extensions;
	void(*qglColorTableEXT)(int, int, int, int, int, const void *);
	qboolean is8bit;
	qboolean isPermedia;
	qboolean gl_mtexable;
	/* gl_vidnt.c */
	lmode_t lowresmodes[];
	const char *gl_vendor;
	const char *gl_renderer;
	const char *gl_version;
	const char *gl_extensions;
	qboolean DDActive;
	qboolean scr_skipupdate;
	int DIBWidth;
	int DIBHeight;
	int vid_modenum;
	int vid_realmode;
	int vid_default;
	unsigned char vid_curpal[256 * 3];
	glvert_t glv;
	cvar_t gl_ztrick;
	viddef_t vid;
	unsigned short d_8to16table[256];
	unsigned d_8to24table[256];
	unsigned char d_15to8table[65536];
	float gldepthmin;
	float gldepthmax;
	modestate_t modestate;
	qboolean is8bit;
	qboolean isPermedia;
	qboolean gl_mtexable;
	cvar_t vid_mode;
	cvar_t _vid_default_mode;
	cvar_t _vid_default_mode_win;
	cvar_t vid_wait;
	cvar_t vid_nopageflip;
	cvar_t _vid_wait_override;
	cvar_t vid_config_x;
	cvar_t vid_config_y;
	cvar_t vid_stretch_by_2;
	cvar_t _windowed_mouse;
	int window_center_x;
	int window_center_y;
	int window_x;
	int window_y;
	int window_width;
	int window_height;
	int texture_mode;
	int texture_extension_number;
	byte scantokey[128];
	byte shiftscantokey[128];
	/* gl_warp.c */
	int skytexturenum;
	int solidskytexture;
	int alphaskytexture;
	float speedscale;
	msurface_t *warpface;
	float turbsin[];
	byte *pcx_rgb;
	TargaHeader targa_header;
	byte *targa_rgba;
	char *suf[6];
	vec3_t skyclip[6];
	int c_sky;
	int st_to_vec[6][3];
	int vec_to_st[6][3];
	float skymins[2][6];
	float skymaxs[2][6];
	int skytexorder[6];
	/* host.c */
	quakeparms_t host_parms;
	qboolean host_initialized;
	double host_frametime;
	double host_time;
	double realtime;
	double oldrealtime;
	int host_framecount;
	int host_hunklevel;
	int minimum_memory;
	client_t *host_client;
	byte *host_basepal;
	byte *host_colormap;
	cvar_t host_framerate;
	cvar_t host_speeds;
	cvar_t sys_ticrate;
	cvar_t serverprofile;
	cvar_t fraglimit;
	cvar_t timelimit;
	cvar_t teamplay;
	cvar_t samelevel;
	cvar_t noexit;
	cvar_t developer;
	cvar_t developer;
	cvar_t skill;
	cvar_t deathmatch;
	cvar_t coop;
	cvar_t pausable;
	cvar_t temp1;
	/* host_cmd.c */
	int current_skill;
	qboolean noclip_anglehack;
	/* in_null.c */
	/* keys.c */
	char key_lines[32][MAXCMDLINE];
	int key_linepos;
	int shift_down;
	int key_lastpress;
	int edit_line;
	int history_line;
	keydest_t key_dest;
	int key_count;
	char *keybindings[256];
	qboolean consolekeys[256];
	qboolean menubound[256];
	int keyshift[256];
	int key_repeats[256];
	qboolean keydown[256];
	keyname_t keynames[];
	char chat_buffer[32];
	qboolean team_message;
	/* mathlib.c */
	vec3_t vec3_origin;
	int nanmask;
	/* menu.c */
	void(*vid_menudrawfn)(void);
	void(*vid_menukeyfn)(int key);
	enum {} m_state;
	qboolean m_entersound;
	qboolean m_recursiveDraw;
	int m_return_state;
	qboolean m_return_onerror;
	char m_return_reason[32];
	byte identityTable[256];
	byte translationTable[256];
	int m_save_demonum;
	int m_main_cursor;
	int m_singleplayer_cursor;
	int load_cursor;
	char m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH + 1];
	int loadable[MAX_SAVEGAMES];
	int m_multiplayer_cursor;
	int setup_cursor;
	int setup_cursor_table[];
	char setup_hostname[16];
	char setup_myname[16];
	int setup_oldtop;
	int setup_oldbottom;
	int setup_top;
	int setup_bottom;
	int m_net_cursor;
	int m_net_items;
	int m_net_saveHeight;
	char *net_helpMessage[];
	int options_cursor;
	char *bindnames[][2];
	int keys_cursor;
	int bind_grab;
	int help_page;
	int msgNumber;
	int m_quit_prevstate;
	qboolean wasInMenus;
	char *quitMessage[];
	int serialConfig_cursor;
	int serialConfig_cursor_table[];
	int serialConfig_baudrate[];
	int serialConfig_comport;
	int serialConfig_irq;
	int serialConfig_baud;
	char serialConfig_phone[16];
	int modemConfig_cursor;
	int modemConfig_cursor_table[];
	char modemConfig_dialing;
	char modemConfig_clear[16];
	char modemConfig_init[32];
	char modemConfig_hangup[16];
	int lanConfig_cursor;
	int lanConfig_cursor_table[];
	int lanConfig_port;
	char lanConfig_portname[6];
	char lanConfig_joinname[22];
	level_t levels[];
	level_t hipnoticlevels[];
	level_t roguelevels[];
	episode_t episodes[];
	episode_t hipnoticepisodes[];
	episode_t rogueepisodes[];
	int startepisode;
	int startlevel;
	int maxplayers;
	qboolean m_serverInfoMessage;
	double m_serverInfoMessageTime;
	int gameoptions_cursor_table[];
	int gameoptions_cursor;
	qboolean searchComplete;
	double searchCompleteTime;
	int slist_cursor;
	qboolean slist_sorted;
	/* model.c */
	model_t *loadmodel;
	char loadname[32];
	byte mod_novis[MAX_MAP_LEAFS / 8];
	model_t mod_known[MAX_MOD_KNOWN];
	int mod_numknown;
	byte *mod_base;
	/* net_loop.c */
	qboolean localconnectpending;
	qsocket_t *loop_client;
	qsocket_t *loop_server;
	/* net_main.c */
	qsocket_t *net_activeSockets;
	qsocket_t *net_freeSockets;
	int net_numsockets;
	qboolean serialAvailable;
	qboolean ipxAvailable;
	qboolean tcpipAvailable;
	int net_hostport;
	int DEFAULTnet_hostport;
	char my_ipx_address[NET_NAMELEN];
	char my_tcpip_address[NET_NAMELEN];
	void(*GetComPortConfig)(int portNumber, int *port, int *irq, int *baud, qboolean *useModem);
	void(*SetComPortConfig)(int portNumber, int port, int irq, int baud, qboolean useModem);
	void(*GetModemConfig)(int portNumber, char *dialType, char *clear, char *init, char *hangup);
	void(*SetModemConfig)(int portNumber, char *dialType, char *clear, char *init, char *hangup);
	qboolean slistInProgress;
	qboolean slistSilent;
	qboolean slistLocal;
	PollProcedure slistSendProcedure;
	PollProcedure slistPollProcedure;
	sizebuf_t net_message;
	int net_activeconnections;
	int messagesSent;
	int messagesReceived;
	int unreliableMessagesSent;
	int unreliableMessagesReceived;
	cvar_t net_messagetimeout;
	cvar_t hostname;
	qboolean configRestored;
	cvar_t config_com_port;
	cvar_t config_com_irq;
	cvar_t config_com_baud;
	cvar_t config_com_modem;
	cvar_t config_modem_dialtype;
	cvar_t config_modem_clear;
	cvar_t config_modem_init;
	cvar_t config_modem_hangup;
	cvar_t idgods;
	int vcrFile;
	qboolean recording;
	int net_driverlevel;
	double net_time;
	int hostCacheCount;
	hostcache_t hostcache[HOSTCACHESIZE];
	struct {} vcrConnect;
	struct {} vcrGetMessage;
	struct {} vcrSendMessage;
	/* net_none.c */
	net_driver_t net_drivers[MAX_NET_DRIVERS];
	int net_numdrivers;
	net_landriver_t net_landrivers[MAX_NET_DRIVERS];
	int net_numlandrivers;
	/* net_vcr.c */
	/* nonintel.c */
	/* pr_cmds.c */
	byte checkpvs[MAX_MAP_LEAFS / 8];
	int c_invis;
	int c_notvis;
	char pr_string_temp[128];
	cvar_t sv_aim;
	builtin_t pr_builtin[];
	builtin_t *pr_builtins;
	int pr_numbuiltins;
	/* pr_edict.c */
	dprograms_t *progs;
	dfunction_t *pr_functions;
	char *pr_strings;
	ddef_t *pr_fielddefs;
	ddef_t *pr_globaldefs;
	dstatement_t *pr_statements;
	float *pr_globals;
	int pr_edict_size;
	unsigned short pr_crc;
	int type_size[8];
	cvar_t nomonsters;
	cvar_t gamecfg;
	cvar_t scratch1;
	cvar_t scratch2;
	cvar_t scratch3;
	cvar_t scratch4;
	cvar_t savedgamecfg;
	cvar_t saved1;
	cvar_t saved2;
	cvar_t saved3;
	cvar_t saved4;
	/* pr_exec.c */
	prstack_t pr_stack[MAX_STACK_DEPTH];
	int pr_depth;
	int localstack[LOCALSTACK_SIZE];
	int localstack_used;
	qboolean pr_trace;
	dfunction_t *pr_xfunction;
	int pr_xstatement;
	int pr_argc;
	char *pr_opnames[];
	/* r_aclip.c */
	int R_AliasClip(finalvert_t *in, finalvert_t *out, int flag, int count, void(*clip)(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out));
	/* r_alias.c */
	mtriangle_t *ptriangles;
	affinetridesc_t r_affinetridesc;
	void *acolormap;
	trivertx_t *r_apverts;
	mdl_t *pmdl;
	vec3_t r_plightvec;
	int r_ambientlight;
	float r_shadelight;
	aliashdr_t *paliashdr;
	finalvert_t *pfinalverts;
	auxvert_t *pauxverts;
	int r_amodels_drawn;
	int a_skinwidth;
	int r_anumverts;
	float aliastransform[3][4];
	float r_avertexnormals[NUMVERTEXNORMALS][3];
	/* r_bsp.c */
	qboolean insubmodel;
	entity_t *currententity;
	vec3_t modelorg;
	vec3_t base_modelorg;
	vec3_t r_entorigin;
	float entity_rotation[3][3];
	vec3_t r_worldmodelorg;
	int r_currentbkey;
	/* r_draw.c */
	unsigned int cacheoffset;
	int c_faceclip;
	zpointdesc_t r_zpointdesc;
	polydesc_t r_polydesc;
	clipplane_t *entity_clipplanes;
	clipplane_t view_clipplanes[4];
	clipplane_t world_clipplanes[16];
	medge_t *r_pedge;
	qboolean r_leftclipped;
	qboolean r_rightclipped;
	qboolean r_nearzionly;
	int sintable[SIN_BUFFER_SIZE];
	int intsintable[SIN_BUFFER_SIZE];
	mvertex_t r_leftenter;
	mvertex_t r_leftexit;
	mvertex_t r_rightenter;
	mvertex_t r_rightexit;
	int r_emitted;
	float r_nearzi;
	float r_u1;
	float r_v1;
	float r_lzi1;
	int r_ceilv1;
	qboolean r_lastvertvalid;
	/* r_edge.c */
	edge_t *r_edges;
	edge_t *edge_p;
	edge_t *edge_max;
	surf_t *surfaces;
	surf_t *surface_p;
	surf_t *surf_max;
	edge_t *newedges[MAXHEIGHT];
	edge_t *removeedges[MAXHEIGHT];
	espan_t *span_p;
	espan_t *max_span_p;
	int r_currentkey;
	int current_iv;
	int edge_head_u_shift20;
	int edge_tail_u_shift20;
	edge_t edge_head;
	edge_t edge_tail;
	edge_t edge_aftertail;
	edge_t edge_sentinel;
	float fv;
	/* r_efrag.c */
	mnode_t *r_pefragtopnode;
	efrag_t **lastlink;
	vec3_t r_emins;
	vec3_t r_emaxs;
	entity_t *r_addent;
	/* r_light.c */
	int r_dlightframecount;
	/* r_main.c */
	void *colormap;
	vec3_t viewlightvec;
	alight_t r_viewlighting;
	float r_time1;
	int r_numallocatededges;
	qboolean r_drawpolys;
	qboolean r_drawculledpolys;
	qboolean r_worldpolysbacktofront;
	qboolean r_recursiveaffinetriangles;
	int r_pixbytes;
	float r_aliasuvscale;
	int r_outofsurfaces;
	int r_outofedges;
	qboolean r_dowarp;
	qboolean r_dowarpold;
	qboolean r_viewchanged;
	int numbtofpolys;
	btofpoly_t *pbtofpolys;
	mvertex_t *r_pcurrentvertbase;
	int c_surf;
	int r_maxsurfsseen;
	int r_maxedgesseen;
	int r_cnumsurfs;
	qboolean r_surfsonstack;
	int r_clipflags;
	byte *r_warpbuffer;
	byte *r_stack_start;
	qboolean r_fov_greater_than_90;
	vec3_t vup;
	vec3_t base_vup;
	vec3_t vpn;
	vec3_t base_vpn;
	vec3_t vright;
	vec3_t base_vright;
	vec3_t r_origin;
	refdef_t r_refdef;
	float xcenter;
	float ycenter;
	float xscale;
	float yscale;
	float xscaleinv;
	float yscaleinv;
	float xscaleshrink;
	float yscaleshrink;
	float aliasxscale;
	float aliasyscale;
	float aliasxcenter;
	float aliasycenter;
	int screenwidth;
	float pixelAspect;
	float screenAspect;
	float verticalFieldOfView;
	float xOrigin;
	float yOrigin;
	mplane_t screenedge[4];
	int r_framecount;
	int r_visframecount;
	int d_spanpixcount;
	int r_polycount;
	int r_drawnpolycount;
	int r_wholepolycount;
	char viewmodname[VIEWMODNAME_LENGTH + 1];
	int modcount;
	int *pfrustum_indexes[4];
	int r_frustum_indexes[4 * 6];
	int reinit_surfcache;
	mleaf_t *r_viewleaf;
	mleaf_t *r_oldviewleaf;
	texture_t *r_notexture_mip;
	float r_aliastransition;
	float r_resfudge;
	int d_lightstylevalue[256];
	float dp_time1;
	float dp_time2;
	float db_time1;
	float db_time2;
	float rw_time1;
	float rw_time2;
	float se_time1;
	float se_time2;
	float de_time1;
	float de_time2;
	float dv_time1;
	float dv_time2;
	cvar_t r_draworder;
	cvar_t r_speeds;
	cvar_t r_timegraph;
	cvar_t r_graphheight;
	cvar_t r_clearcolor;
	cvar_t r_waterwarp;
	cvar_t r_fullbright;
	cvar_t r_drawentities;
	cvar_t r_drawviewmodel;
	cvar_t r_aliasstats;
	cvar_t r_dspeeds;
	cvar_t r_drawflat;
	cvar_t r_ambient;
	cvar_t r_reportsurfout;
	cvar_t r_maxsurfs;
	cvar_t r_numsurfs;
	cvar_t r_reportedgeout;
	cvar_t r_maxedges;
	cvar_t r_numedges;
	cvar_t r_aliastransbase;
	cvar_t r_aliastransadj;
	/* r_misc.c */
	/* r_part.c */
	int ramp1[8];
	int ramp2[8];
	int ramp3[8];
	particle_t *active_particles;
	particle_t *free_particles;
	particle_t *particles;
	int r_numparticles;
	vec3_t r_pright;
	vec3_t r_pup;
	vec3_t r_ppn;
	vec3_t avelocities[NUMVERTEXNORMALS];
	float beamlength;
	vec3_t avelocity;
	float partstep;
	float timescale;
	/* r_sky.c */
	int iskyspeed;
	int iskyspeed2;
	float skyspeed;
	float skyspeed2;
	float skytime;
	byte *r_skysource;
	int r_skymade;
	int r_skydirect;
	byte bottomsky[128 * 131];
	byte bottommask[128 * 131];
	byte newsky[128 * 256];
	/* r_sprite.c */
	spritedesc_t r_spritedesc;
	/* r_surf.c */
	drawsurf_t r_drawsurf;
	int lightleft;
	int sourcesstep;
	int blocksize;
	int sourcetstep;
	int lightdelta;
	int lightdeltastep;
	int lightright;
	int lightleftstep;
	int lightrightstep;
	int blockdivshift;
	unsigned blockdivmask;
	void *prowdestbase;
	unsigned char *pbasesource;
	int surfrowbytes;
	unsigned *r_lightptr;
	int r_stepback;
	int r_lightwidth;
	int r_numhblocks;
	int r_numvblocks;
	unsigned char *r_source;
	unsigned char *r_sourcemax;
	unsigned blocklights[18 * 18];
	/* r_vars.c */
	int r_bmodelactive;
	/* sbar.c */
	int sb_updates;
	qpic_t *sb_nums[2][11];
	qpic_t *sb_colon;
	qpic_t *sb_slash;
	qpic_t *sb_ibar;
	qpic_t *sb_sbar;
	qpic_t *sb_scorebar;
	qpic_t *sb_weapons[7][8];
	qpic_t *sb_ammo[4];
	qpic_t *sb_sigil[4];
	qpic_t *sb_armor[3];
	qpic_t *sb_items[32];
	qpic_t *sb_faces[7][2];
	qpic_t *sb_face_invis;
	qpic_t *sb_face_quad;
	qpic_t *sb_face_invuln;
	qpic_t *sb_face_invis_invuln;
	qboolean sb_showscores;
	int sb_lines;
	qpic_t *rsb_invbar[2];
	qpic_t *rsb_weapons[5];
	qpic_t *rsb_items[2];
	qpic_t *rsb_ammo[3];
	qpic_t *rsb_teambord;
	qpic_t *hsb_weapons[7][5];
	int hipweapons[4];
	qpic_t *hsb_items[2];
	int fragsort[MAX_SCOREBOARD];
	char scoreboardtext[MAX_SCOREBOARD][20];
	int scoreboardtop[MAX_SCOREBOARD];
	int scoreboardbottom[MAX_SCOREBOARD];
	int scoreboardcount[MAX_SCOREBOARD];
	int scoreboardlines;
	/* screen.c */
	int scr_copytop;
	int scr_copyeverything;
	float scr_con_current;
	float scr_conlines;
	float oldscreensize;
	float oldfov;
	cvar_t scr_viewsize;
	cvar_t scr_fov;
	cvar_t scr_conspeed;
	cvar_t scr_centertime;
	cvar_t scr_showram;
	cvar_t scr_showturtle;
	cvar_t scr_showpause;
	cvar_t scr_printspeed;
	qboolean scr_initialized;
	qpic_t *scr_ram;
	qpic_t *scr_net;
	qpic_t *scr_turtle;
	int scr_fullupdate;
	int clearconsole;
	int clearnotify;
	viddef_t vid;
	vrect_t *pconupdate;
	vrect_t scr_vrect;
	qboolean scr_disabled_for_loading;
	qboolean scr_drawloading;
	float scr_disabled_time;
	qboolean scr_skipupdate;
	qboolean block_drawing;
	char scr_centerstring[1024];
	float scr_centertime_start;
	float scr_centertime_off;
	int scr_center_lines;
	int scr_erase_lines;
	int scr_erase_center;
	char *scr_notifystring;
	qboolean scr_drawdialog;
	/* snd_mem.c */
	int cache_full_cycle;
	byte *data_p;
	byte *iff_end;
	byte *last_chunk;
	byte *iff_data;
	int iff_chunk_len;
	/* snd_mix.c */
	portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
	int snd_scaletable[32][256];
	int *snd_p;
	int snd_linear_count;
	int snd_vol;
	short *snd_out;
	/* snd_next.c */
	/* snd_null.c */
	cvar_t bgmvolume;
	cvar_t volume;
	/* sv_main.c */
	server_t sv;
	server_static_t svs;
	char localmodels[MAX_MODELS][5];
	int fatbytes;
	byte fatpvs[MAX_MAP_LEAFS / 8];
	/* sv_move.c */
	int c_yes;
	int c_no;
	/* sv_phys.c */
	cvar_t sv_friction;
	cvar_t sv_stopspeed;
	cvar_t sv_gravity;
	cvar_t sv_maxvelocity;
	cvar_t sv_nostep;
	/* sv_user.c */
	edict_t *sv_player;
	cvar_t sv_edgefriction;
	vec3_t wishdir;
	float wishspeed;
	float *angles;
	float *origin;
	float *velocity;
	qboolean onground;
	usercmd_t cmd;
	cvar_t sv_idealpitchscale;
	cvar_t sv_maxspeed;
	cvar_t sv_accelerate;
	/* sys_null.c */
	qboolean isDedicated;
	/* vid_null.c */
	/* view.c */
	cvar_t lcd_x;
	cvar_t lcd_yaw;
	cvar_t scr_ofsx;
	cvar_t scr_ofsy;
	cvar_t scr_ofsz;
	cvar_t cl_rollspeed;
	cvar_t cl_rollangle;
	cvar_t cl_bob;
	cvar_t cl_bobcycle;
	cvar_t cl_bobup;
	cvar_t v_kicktime;
	cvar_t v_kickroll;
	cvar_t v_kickpitch;
	cvar_t v_iyaw_cycle;
	cvar_t v_iroll_cycle;
	cvar_t v_ipitch_cycle;
	cvar_t v_iyaw_level;
	cvar_t v_iroll_level;
	cvar_t v_ipitch_level;
	cvar_t v_idlescale;
	cvar_t crosshair;
	cvar_t cl_crossx;
	cvar_t cl_crossy;
	cvar_t gl_cshiftpercent;
	float v_dmg_time;
	float v_dmg_roll;
	float v_dmg_pitch;
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	cvar_t v_centermove;
	cvar_t v_centerspeed;
	cshift_t cshift_empty;
	cshift_t cshift_water;
	cshift_t cshift_slime;
	cshift_t cshift_lava;
	cvar_t v_gamma;
	byte gammatable[256];
	byte ramps[3][256];
	float v_blend[4];
	/* wad.c */
	int wad_numlumps;
	lumpinfo_t *wad_lumps;
	byte *wad_base;
	/* world.c */
	/* zone.c */
	memzone_t *mainzone;
	byte *hunk_base;
	int hunk_size;
	int hunk_low_used;
	int hunk_high_used;
	qboolean hunk_tempactive;
	int hunk_tempmark;
	cache_system_t cache_head;
} quake_state_t;

extern quake_state_t* G;

#endif