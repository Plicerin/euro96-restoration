#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include "graphics_data.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
namespace actua::legacy {
void Core::camera(float x,float y,float z,float tx,float ty,float tz,float distance,int width,int height){
    // Camera construction from render3d; window/viewport setup replaces DOS buffers.
    eye_x=x;eye_y=y;eye_z=-z;
    z=-z;tz=-tz;tx-=x;ty-=y;tz-=z;
    double hr=tx*tx+tz*tz;if(hr<1){hr=1;tz=1;}
    const double r=std::sqrt(hr+ty*ty);hr=std::sqrt(hr);
    const float cth=float(tz/hr),sth=float(tx/hr),cph=float(hr/r),sph=float(ty/r);
    mat unit{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}},tmp;
    matcop(rot.arr,unit);matcop(tran.arr,unit);matcop(tmp,unit);
    rot.arr[0][3]=-x;rot.arr[1][3]=-y;rot.arr[2][3]=-z;
    tmp[0][0]=cth;tmp[0][2]=-sth;tmp[1][0]=-sth*sph;tmp[1][1]=cph;
    tmp[1][2]=-cth*sph;tmp[2][0]=sth*cph;tmp[2][1]=sph;tmp[2][2]=cth*cph;
    matmul(rot.arr,tmp);q=distance;qa=q*2/100;persc=q/15;render_buff.clip_xmid=width/2;render_buff.clip_ymid=height/2;render_buff.clip_wid=width;render_buff.clip_hgt=height;
    clear();
}
}
namespace actua {
static void checked(bool value){if(!value)throw std::runtime_error(SDL_GetError());}
OriginalGraphics::OriginalGraphics(const std::filesystem::path& data):archive(data/"EUROREND.DAT",data/"EUROREND.OFF"),core(std::make_unique<legacy::Core>()){
    for(int record=134;record<229;++record){
        auto raw=archive.entry(std::size_t(record));if(raw.size()%340)throw std::runtime_error("Invalid motion capture size");
        OriginalClip c;c.record=record;c.name="Capture "+std::to_string(record);c.frames.resize(raw.size()/340);
        std::memcpy(c.frames.data(),raw.data(),raw.size());
        for(const auto& f:c.frames){if(f[0]!=28)throw std::runtime_error("Invalid motion point count");for(float v:f)if(!std::isfinite(v)||std::abs(v)>10000)throw std::runtime_error("Invalid motion point");}
        for(const auto& m:recovered::motions)if(m[1]==record){
            if(std::size_t(m[2])!=c.frames.size())throw std::runtime_error("Motion archive/object revision mismatch");c.animation=m[0];
        }
        switch(c.animation){case 34:c.name="Shoot";break;case 36:c.name="Chip";break;case 38:c.name="Short pass";break;case 72:c.name="Run";break;case 73:c.name="Jog";break;case 78:c.name="Stand";break;case 85:c.name="Sliding tackle";break;case 86:c.name="Steal";break;case 98:c.name="Keeper kick";break;default:break;}
        clips.push_back(std::move(c));
    }
    video_board=std::make_unique<OriginalVideoBoard>(data);
    environment=std::make_unique<OriginalEnvironment>(archive);
    overlay=std::make_unique<OriginalOverlay>(archive);
    sequence=std::make_unique<OriginalActions>(clips);
    const Archive euro(data/"EURO.DAT",data/"EURO.OFF");
    sequence->set_team_tactics(decode_tactics(euro.entry(111)));
    std::array<Tactics,10> formations;for(int i=0;i<10;++i)formations[i]=decode_tactics(euro.entry(111+i));
    sequence->set_match_tactics(formations);
    // Original init3d extends the pitch mesh; keep dimensions local to this renderer.
    pitch_points.assign(std::begin(recovered::pitch_p),std::end(recovered::pitch_p));
    for(int i=0;i<19;++i){pitch_points[1+i*3+2]=float(recovered::stad_width);pitch_points[1+(i+19)*3+2]=-800.0f-recovered::stad_width;}
    pitch_points[1]=pitch_points[1+19*3]=-float(recovered::stad_length);
    pitch_points[1+18*3]=pitch_points[1+37*3]=1280.0f+recovered::stad_length;
    stadium_width=recovered::stad_width;stadium_length=recovered::stad_length;stadium_height=recovered::stad_height;
    for(int i=0;i<4;++i)stand_positions[i]={recovered::stands[i].x,recovered::stands[i].y,recovered::stands[i].z};
    auto raw=archive.entry(1);if(raw.size()%32)throw std::runtime_error("Invalid texture coordinates");
    texture_coords.resize(raw.size()/32+1);std::memcpy(texture_coords.data()+1,raw.data(),raw.size());
    auto standtex=archive.entry(125);texture_coords.resize(1004+standtex.size()/32);std::memcpy(texture_coords.data()+1004,standtex.data(),standtex.size());
    for(auto& t:texture_coords)for(auto& v:t)v&=0xffffff00;
    if(texture_coords.size()+2>=2000)throw std::runtime_error("Original texture pool too large");
    std::memcpy(core->textures,texture_coords.data(),texture_coords.size()*32);
    core->clip_base=core->clip_tex=int(texture_coords.size());
    for(std::size_t i=0;i<4;++i){
        auto points=archive.entry(recovered::stands[i].points),faces=archive.entry(recovered::stands[i].faces);
        if(points.size()%4 || faces.size()%2)throw std::runtime_error("Invalid original stadium mesh");
        stands[i].points.resize(points.size()/4);stands[i].faces.resize(faces.size()/2);
        std::memcpy(stands[i].points.data(),points.data(),points.size());std::memcpy(stands[i].faces.data(),faces.data(),faces.size());
        if(stands[i].points.empty() || stands[i].faces.empty() || stands[i].points[0]<1 || stands[i].points[0]>500 || stands[i].points[0]*3+1!=stands[i].points.size())throw std::runtime_error("Stadium point count mismatch");
        for(float v:stands[i].points)if(!std::isfinite(v) || std::abs(v)>10000)throw std::runtime_error("Invalid stadium coordinate");
        const auto& faceset=stands[i].faces;std::size_t at=1;
        if(faceset[0]<0 || faceset[0]>200)throw std::runtime_error("Invalid stadium face count");
        for(int face=0;face<faceset[0];++face){
            if(at+2>faceset.size())throw std::runtime_error("Truncated stadium face");const int count=faceset[at];if(stands[i].faces[at+1]<0)stands[i].faces[at+1]-=1003;at+=2; // Original init3d: rebase stadium maps by S_TM-1.
            if(count<2 || count>16 || at+std::size_t(count)>faceset.size())throw std::runtime_error("Unsupported stadium face");
            for(int j=0;j<count;++j)if(faceset[at+j]<0 || faceset[at+j]>=stands[i].points[0])throw std::runtime_error("Stadium point index out of range");at+=count;
        }
        if(at!=faceset.size())throw std::runtime_error("Trailing stadium data");
    }
}
OriginalGraphics::~OriginalGraphics(){SDL_DestroyTexture(environment_texture);SDL_DestroyTexture(overlay_texture);for(auto* t:textures)SDL_DestroyTexture(t);}
void OriginalGraphics::change_clip(int delta){clip=(clip+delta+int(clips.size()))%int(clips.size());frame=0;clock=0;}
void OriginalGraphics::change_kit(int delta){kit=(kit+delta+16)%16;for(auto*& t:textures){SDL_DestroyTexture(t);t=nullptr;}texture_owner=nullptr;}
void OriginalGraphics::start_prematch(){
 if(prematches_enabled && has_cd_stadium()){
  finish_replay();exit.finish();clock=0;opening_input_guard=true;
  sequence->clear_play_input();prematch.start(sequence->engine);
 }
}
void OriginalGraphics::finish_prematch(){
 prematch.finish();clock=0;paused=false;opening_input_guard=true;sequence->clear_play_input();
 camera_generation=~std::uint64_t(0);presentation_generation=0;
}
void OriginalGraphics::advance_prematch(double seconds){
 if(prematch.paused)return;
 prematch.advance(seconds);video_board->advance(seconds);
 if(!prematch.active())finish_prematch();
}
bool OriginalGraphics::start_walkoff(){
 if(sequence->walkoff_events.empty())return false;
 if(!walkoffs_enabled || !has_cd_stadium() || sequence->full_time || sequence->interval_ticks){sequence->walkoff_events.clear();return false;}
 const auto event=sequence->walkoff_events.front();sequence->walkoff_events.pop_front();
 finish_replay();clock=0;sequence->clear_play_input();opening_input_guard=true;
 walkoff.start(sequence->engine,event,tunnel_x,tunnel_y);walkoff_generation=sequence->generation;return true;
}
void OriginalGraphics::finish_walkoff(){
 walkoff.finish();clock=0;paused=false;opening_input_guard=true;sequence->clear_play_input();
 camera_generation=~std::uint64_t(0);presentation_generation=0;
 start_walkoff();
}
void OriginalGraphics::start_exit(){
 if(exits_enabled && has_cd_stadium()){finish_replay();exit.start(sequence->engine,tunnel_x,tunnel_y);}
}
void OriginalGraphics::advance(double seconds){
    if(walkoff.active() && walkoff_generation!=sequence->generation)walkoff.finish();
    if(walkoff.active()){
        if(paused)return;
        walkoff.advance(seconds);
        if(!walkoff.active())finish_walkoff();
        return;
    }
    if(exit.active() || prematch.active())return;
    if(action_view && presentation_generation!=sequence->generation){clock=0;presentation_generation=sequence->generation;presentation.observe(sequence->engine,sequence->generation,sequence->ticks);}
    if(paused)return;
    if(!replaying && action_view && start_walkoff())return;
    clock+=std::clamp(seconds,0.0,0.1)*20*(replaying?replay_rate/5.0:1.0);
    while(clock>=1){
        if(action_view){
            if(replaying){
                if(++replay_cursor>=int(history.size())){
                    if(replay_automatic){finish_replay();return;}
                    seek_replay(0);paused=!replay_loop;return;
                }
                load_replay_frame();step_camera();replay_presentation.observe(*replay_engine,1,replay_cursor);
            }else if(sequence->full_match && sequence->engine.just_scored==1 && replayed_goals!=sequence->engine.goal_events && start_replay(true)){
                // Hold the live match at the end of its celebration, then resume its kickoff.
                return;
            }else{sequence->step();video_board->observe(*sequence);video_board->advance(.05);step_camera();presentation.observe(sequence->engine,sequence->generation,sequence->ticks);record_replay();}
        }
        else frame=(frame+1)%int(clips[std::size_t(clip)].frames.size());
        clock-=1;
        if(!replaying && action_view && start_walkoff())return;
    }
}

void OriginalGraphics::load_cd_graphics(const std::filesystem::path& root){
 video_board->load_retail(root);
 media_root=root;cd_kits.load(root);euro_kits.load(root);shirt_numbers.load(root);euro_players.load(root);if(stadium_available(5))stadium_index=next_stadium=5;load_cd_stadium(root,stadium_index);bind_media_textures();sequence->walkoff_events_enabled=has_cd_stadium();texture_home=texture_away=-1;
}
void OriginalGraphics::bind_media_textures(){
 const auto original_uv=archive.entry(1);
 std::memcpy(texture_coords.data()+1,original_uv.data(),120*32);
 std::memcpy(texture_coords.data()+761,original_uv.data()+760*32,60*32);
 for(int i=1;i<821;++i)if(i<=120 || i>=761)for(auto& v:texture_coords[i])v&=0xffffff00;
 if(retail_player_textures)euro_players.bind(texture_coords);
 if(number_texture_base && texture_coords.size()==std::size_t(number_texture_base+40))texture_coords.resize(number_texture_base);
 number_texture_base=0;
 if(shirt_numbers.loaded()){
  if(texture_coords.size()+42>=std::size(core->textures))throw std::runtime_error("No texture-coordinate space for retail shirt numbers");
  number_texture_base=int(texture_coords.size());
  const auto& coords=shirt_numbers.coordinates();texture_coords.insert(texture_coords.end(),coords.begin(),coords.end());
 }
 std::memcpy(core->textures,texture_coords.data(),texture_coords.size()*32);core->clip_base=core->clip_tex=int(texture_coords.size());
}
namespace {constexpr std::array<const char*,13> venue_files{"STADIUM.BIN","SCOTLAND.BIN","SPAIN.BIN","ITALY.BIN","BRAZIL.BIN","EURO_STADIUM0.BIN","EURO_STADIUM1.BIN","EURO_STADIUM2.BIN","EURO_STADIUM3.BIN","EURO_STADIUM4.BIN","EURO_STADIUM5.BIN","EURO_STADIUM6.BIN","EURO_STADIUM7.BIN"};}
bool OriginalGraphics::stadium_available(int index)const{return index>=0 && index<int(venue_files.size()) && !media_root.empty() && std::filesystem::exists(media_root/"GRAPHICS"/venue_files[index]);}
void OriginalGraphics::change_stadium(int delta){const int first=stadium_available(5)?5:0,count=first?8:5;int candidate=next_stadium;if(candidate<first || candidate>=first+count)candidate=delta<0?first:first-1;for(int n=0;n<count;++n){candidate=first+(candidate-first+delta+count)%count;if(stadium_available(candidate)){next_stadium=candidate;return;}}next_stadium=stadium_index;}
void OriginalGraphics::apply_stadium(){if(next_stadium!=stadium_index && stadium_available(next_stadium)){load_cd_stadium(media_root,next_stadium);stadium_index=next_stadium;texture_home=texture_away=-1;video_revision=~0u;}}
void OriginalGraphics::load_cd_stadium(const std::filesystem::path& root,int index){
 const auto path=root/"GRAPHICS"/venue_files.at(index);if(!std::filesystem::exists(path))return;
 if(std::filesystem::file_size(path)>1024*1024)throw std::runtime_error("Oversized CD stadium");
 const auto bytes=read_file(path);
 if(bytes.size()<248)throw std::runtime_error("Invalid CD stadium header");
 const auto signature=std::string(bytes.begin(),bytes.begin()+8);const bool retail=signature=="EU96STD1";
 if(signature!="AC96STD1" && !retail)throw std::runtime_error("Invalid CD stadium signature");
 MatchPalette imported_palette{};if(retail){if(bytes.size()<1016)throw std::runtime_error("Truncated retail stadium palette");std::memcpy(imported_palette.data(),bytes.data()+248,768);for(const auto& c:imported_palette)for(auto v:c)if(v>63)throw std::runtime_error("Invalid retail stadium palette");}
 const auto word=[&](std::size_t at){if(at+4>bytes.size())throw std::runtime_error("Truncated CD stadium");unsigned v=0;for(int i=0;i<4;++i)v|=unsigned(bytes[at+i])<<(8*i);return v;};
 const int width=int(word(112)),length=int(word(116)),height=int(word(120));
 if(width<0 || width>2000 || length<0 || length>2000 || height<1 || height>2000)throw std::runtime_error("Invalid CD stadium bounds");
 std::array<Bytes,15> parts;std::size_t at=retail?1016:248;
 for(auto& part:parts){const auto size=word(at);at+=4;if(size>bytes.size()-at)throw std::runtime_error("Truncated CD stadium component");part.assign(bytes.begin()+at,bytes.begin()+at+size);at+=size;}
 if(at!=bytes.size() || parts[0].empty() || parts[0].size()%32 || parts[0].size()/32+1006>=2000 || parts[1].size()!=65536 || parts[2].size()!=65536 || parts[11].size()!=16384 || parts[12].size()!=48 || parts[13].size()!=307200 || parts[14].size()!=48)throw std::runtime_error("Invalid CD stadium components");
 for(int i:{12,14})if(*std::max_element(parts[i].begin(),parts[i].end())>63)throw std::runtime_error("Invalid CD stadium palette");
 std::array<StandMesh,4> meshes;std::array<std::array<float,3>,4> positions;
 for(int i=0;i<4;++i){
  auto& m=meshes[i];const auto& p=parts[3+i*2];const auto& f=parts[4+i*2];
  if(p.empty() || p.size()%4 || f.size()<2 || f.size()%2)throw std::runtime_error("Invalid CD stadium mesh");
  m.points.resize(p.size()/4);m.faces.resize(f.size()/2);std::memcpy(m.points.data(),p.data(),p.size());std::memcpy(m.faces.data(),f.data(),f.size());
  if(m.points[0]<1 || m.points[0]>500 || m.points[0]*3+1!=m.points.size() || m.faces[0]<1 || m.faces[0]>200)throw std::runtime_error("Invalid CD stadium mesh counts");
  for(float v:m.points)if(!std::isfinite(v) || std::abs(v)>10000)throw std::runtime_error("Invalid CD stadium point");
  std::size_t cursor=1;
  for(int j=0;j<m.faces[0];++j){
   if(cursor+2>m.faces.size())throw std::runtime_error("Truncated CD stadium face");const int n=m.faces[cursor],material=m.faces[cursor+1];
   if(n<2 || n>16 || cursor+2+n>m.faces.size())throw std::runtime_error("Invalid CD stadium polygon");
   if(material<0){const int id=material<-2000?-material-2000:-material;if(id<1 || id>parts[0].size()/32)throw std::runtime_error("Invalid CD stadium material");m.faces[cursor+1]-=1003;}
   else if(material>257)throw std::runtime_error("Unsupported CD stadium material");
   for(int k=0;k<n;++k)if(m.faces[cursor+2+k]<0 || m.faces[cursor+2+k]>=m.points[0])throw std::runtime_error("Invalid CD stadium face index");
   cursor+=2+n;
  }
  if(cursor!=m.faces.size())throw std::runtime_error("Trailing CD stadium faces");
  std::memcpy(positions[i].data(),bytes.data()+8+24+i*20,12);
  for(float v:positions[i])if(!std::isfinite(v) || std::abs(v)>10000)throw std::runtime_error("Invalid CD stadium position");
 }
 auto coords=texture_coords;coords.resize(1004+parts[0].size()/32);std::memcpy(coords.data()+1004,parts[0].data(),parts[0].size());
 for(std::size_t i=1004;i<coords.size();++i){for(auto& v:coords[i])v&=0xffffff00;const auto page=coords[i][0]>>24;if(page!=8 && page!=9)throw std::runtime_error("Invalid CD stadium texture page");}
 const int raw_video=int(word(244)),page=retail?raw_video>>16:8,offset=retail?raw_video&65535:raw_video;
 if((page!=8 && page!=9 && page!=15) || offset<0 || offset%256+128>256 || offset/256+80>256)throw std::runtime_error("Invalid stadium video rectangle");
 const float tx=float(std::int32_t(word(232))),ty=-float(std::int32_t(word(240)));
 if(tx<-2000 || tx>3280 || ty<-2000 || ty>2800)throw std::runtime_error("Invalid stadium tunnel");
 environment->set_stadium(parts[11],parts[13],width,length,height);
 retail_stadium=retail;stadium_palette=imported_palette;video_page=page;video_offset=offset;
 video_board->use_retail(retail);
 stands=std::move(meshes);stand_positions=positions;stadium_parts=std::move(parts);texture_coords=std::move(coords);
 stadium_width=width;stadium_length=length;stadium_height=height;
 tunnel_x=tx;tunnel_y=ty;
 for(int i=0;i<19;++i){pitch_points[1+i*3+2]=float(width);pitch_points[1+(i+19)*3+2]=-800.0f-width;}
 pitch_points[1]=pitch_points[1+19*3]=-float(length);pitch_points[1+18*3]=pitch_points[1+37*3]=1280.0f+length;
 number_texture_base=0;bind_media_textures();
 camera_generation=~std::uint64_t(0);
}
bool OriginalGraphics::has_original_kit(int team)const{return team>=0 && team<16 && (euro_kits.loaded() || original_teams[team].kit>=0 || cd_kits.has(team));}
void OriginalGraphics::load_textures(SDL_Renderer* renderer){
    const int home_kit=sequence->national_teams?std::max(original_teams[sequence->home_team].kit,0):kit;
    const int away_kit=sequence->national_teams?(original_teams[sequence->away_team].kit<0?5:original_teams[sequence->away_team].kit):(kit+1)%16;
    const bool cd_home=sequence->national_teams && cd_kits.has(sequence->home_team),cd_away=sequence->national_teams && cd_kits.has(sequence->away_team);
    const int alternate=cd_away?cd_kits.away_kit(sequence->home_team,sequence->away_team):sequence->away_team;
    const bool euro=sequence->national_teams && euro_kits.loaded();
    const bool retail_players=euro && euro_players.loaded();
    if(retail_player_textures!=retail_players){retail_player_textures=retail_players;bind_media_textures();texture_home=texture_away=-1;}
    const int home_id=euro?256+sequence->home_team:cd_home?32+sequence->home_team:home_kit,away_id=euro?256+euro_kits.selection(sequence->home_team,sequence->away_team,true):cd_away?32+sequence->away_team+64*(alternate>=16?alternate-15:0):away_kit;
    if(texture_owner==renderer && texture_home==home_id && texture_away==away_id)return;
    for(auto*& texture:textures){SDL_DestroyTexture(texture);texture=nullptr;}
    texture_home=home_id;texture_away=away_id;
    auto pal=archive.entry(0);if(pal.size()!=768)throw std::runtime_error("Invalid renderer palette");std::memcpy(palette.data(),pal.data(),768);
    const auto& k=recovered::kits[home_kit];
    const auto& opponent=recovered::kits[away_kit];
    const auto part=[&](bool away,int field)->std::span<const unsigned char>{
        // Retail Euro96 record 107 is byte-identical to preserved record 109.
        // Its neutral numbers must not inherit the older Actua kit's digit colours.
        if(euro && field==3)return archive.entry(109);
        if(away?cd_away:cd_home)return cd_kits.part(away?((field>=1 && field<=4)?alternate:sequence->away_team):sequence->home_team,field);
        return archive.entry((away?opponent:k)[field]);
    };
    auto kitpal=part(false,4),skin=part(false,5);
    if(kitpal.size()!=72 || skin.size()!=24)throw std::runtime_error("Invalid kit palette");
    auto pitchpal=has_cd_stadium()?std::span<const unsigned char>(stadium_parts[12]):archive.entry(68);if(pitchpal.size()!=48)throw std::runtime_error("Invalid pitch palette");std::memcpy(palette[128].data(),pitchpal.data(),48);
    std::memcpy(palette[32].data(),kitpal.data(),72);std::memcpy(palette[80].data(),skin.data(),24);
    auto opponent_pal=part(true,4),opponent_skin=part(true,5);
    if(opponent_pal.size()!=72 || opponent_skin.size()!=24)throw std::runtime_error("Invalid opponent palette");
    std::memcpy(palette[56].data(),opponent_pal.data(),72);std::memcpy(palette[88].data(),opponent_skin.data(),24);
    auto home=part(false,6),away=part(true,7);
    if(home.size()!=24 || away.size()!=24)throw std::runtime_error("Invalid crowd palette");
    std::memcpy(palette[224].data(),home.data(),24);std::memcpy(palette[232].data(),away.data(),24);
    auto sky_pal=has_cd_stadium()?std::span<const unsigned char>(stadium_parts[14]):archive.entry(recovered::sky_palette);if(sky_pal.size()!=48)throw std::runtime_error("Invalid original sky palette");
    std::memcpy(palette[208].data(),sky_pal.data(),48);
    palette[1]={63,63,63};palette[191]={0,0,0}; // Original screen-mode palette overrides.
    std::array<Bytes,16> pages;for(auto& p:pages)p.resize(65536);
    const auto copy=[&](unsigned record,int page,std::size_t offset){auto b=archive.entry(record);if(b.size()>65536-offset)throw std::runtime_error("Texture overflow");std::copy(b.begin(),b.end(),pages[std::size_t(page)].begin()+offset);};
    const auto kitcopy=[&](bool away,int field,int page,std::size_t offset){auto b=part(away,field);if(b.size()>65536-offset)throw std::runtime_error("CD kit page overflow");std::copy(b.begin(),b.end(),pages[page].begin()+offset);};
    kitcopy(false,0,0,0);kitcopy(true,0,0,128*256);
    kitcopy(false,1,1,0);kitcopy(true,1,2,0);
    kitcopy(false,2,3,0);kitcopy(true,2,3,80*256);copy(34,3,158*256);
    copy(35,4,0);copy(108,5,0); // Verified Euro init3d BM_REFKPTOR/BM_REFKPLIM calls.
    // init3d's M8 appearance pages: preserve the alternate head artwork below
    // the two 78-row limb strips in BM_XLIMBS before applying skin remaps.
    kitcopy(false,1,10,0);kitcopy(true,1,11,0);
    copy(38,12,0);kitcopy(false,2,12,0);kitcopy(true,2,12,80*256);
    copy(39,13,0);copy(35,14,0); // Original away keeper uses the extended mesh texture pages.
    copy(40,15,0); // Original BM_NETS page for near-goal detail levels.
    // Retail ground pages no longer contain the preserved renderer's flag UVs.
    // Keep its verified flag atlas independently of the retail ground tile.
    const auto flag_atlas=has_cd_stadium() && !retail_stadium?std::span<const unsigned char>(stadium_parts[11]):archive.entry(recovered::pitch_bitmap);
    std::copy(flag_atlas.begin(),flag_atlas.end(),pages[6].begin()+116*256);
    copy(36,6,0); // BM_EXTRA1: selection markers, before the shirt-number overlays.
    // init3d places the first eleven numbers in 27-row home/away strips.
    // Original four substitute numbers occupy the shared 116-row strip.
    for(int side=0;side<2;++side){
        auto numbers=part(side!=0,3);
        if(numbers.size()!=54*256)throw std::runtime_error("Invalid original shirt-number atlas");
        const std::size_t start=(side?89:62)*256;
        std::copy_n(numbers.begin(),27*256,pages[6].begin()+start);
        // Original init3d relocates number 12-15 strips from the second atlas row.
        for(int row=0;row<27;++row)std::copy_n(numbers.begin()+(row+27)*256+161,92,pages[6].begin()+(row+116)*256+(side?69:161));
        if(side)for(int row=0;row<27;++row)for(int col=69;col<161;++col){auto& value=pages[6][(row+116)*256+col];if(value>32 && value<=56)value+=24;if(value>80 && value<=88)value+=8;}
        if(side)for(std::size_t i=start;i<start+27*256;++i){auto& value=pages[6][i];if(value>32 && value<=56)value+=24;if(value>80 && value<=88)value+=8;}
    }
    // Original init3d remaps only the away regions, after the feet overlay.
    const auto remap=[](unsigned char& value,bool uniform){if(uniform && value>=32 && value<56)value+=24;if(value>=80 && value<88)value+=8;};
    for(std::size_t i=128*256;i<65536;++i)remap(pages[0][i],false);
    for(auto& value:pages[2])remap(value,true);
    for(std::size_t i=80*256;i<160*256;++i)remap(pages[3][i],true);
    const auto alternate_remap=[](unsigned char& value,bool uniform){if(uniform && value>=32 && value<56)value+=24;if(value>=80 && value<88)value+=16;};
    for(auto& value:pages[10])alternate_remap(value,false);
    for(auto& value:pages[11])alternate_remap(value,true);
    for(std::size_t i=0;i<78*256;++i)alternate_remap(pages[12][i],false);
    for(std::size_t i=80*256;i<158*256;++i)alternate_remap(pages[12][i],true);
    for(int page:{13,14})for(auto& value:pages[page])remap(value,false);
    if(has_cd_stadium()){pages[8]=stadium_parts[1];pages[9]=stadium_parts[2];}else{copy(41,8,0);copy(42,9,0);}
    for(std::size_t p=0;p<textures.size();++p){
        if(p==16 && !shirt_numbers.loaded())continue;
        Bytes rgba(65536*4);for(std::size_t i=0;i<65536;++i){
            // Number assets on the extra-map page already carry polyt's +1
            // encoding. RENDER.OBJ decrements each byte; resulting zero is transparent.
            const auto idx=static_cast<unsigned char>((p==16?0:pages[p][i])-(p==6?1:0));
            for(int c=0;c<3;++c)rgba[i*4+c]=static_cast<unsigned char>(palette[idx][c]*4);rgba[i*4+3]=idx?255:0;
        }
        if(p==16)rgba=shirt_numbers.rgba();
        else {
            if(retail_players)euro_players.overlay(rgba,int(p),sequence->home_team,sequence->away_team);
            if(euro)euro_kits.overlay(rgba,int(p),sequence->home_team,sequence->away_team,retail_players?euro_players.skin_shift(sequence->home_team):-1,retail_players?euro_players.skin_shift(sequence->away_team):-1);
        }
        if(retail_stadium && (p==8 || p==9)){for(std::size_t i=0;i<65536;++i){const auto idx=pages[p][i];for(int c=0;c<3;++c)rgba[i*4+c]=stadium_palette[idx][c]*4;rgba[i*4+3]=idx?255:0;}}
        if(p==1 || p==2){
            // Native contrast choice, not a claimed recovered DOS colour table.
            // Sample the torso-view row rather than shorts, heads or skin pages.
            std::uint64_t light=0,count=0;
            for(int y=44;y<85;++y)for(int x=0;x<256;++x){const auto i=(y*256+x)*4;if(rgba[i+3]){light+=299u*rgba[i]+587u*rgba[i+1]+114u*rgba[i+2];++count;}}
            dark_numbers[p-1]=count && light>count*128000;
        }
        textures[p]=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,256,256);if(!textures[p])throw std::runtime_error(SDL_GetError());
        checked(SDL_UpdateTexture(textures[p],nullptr,rgba.data(),1024));checked(SDL_SetTextureBlendMode(textures[p],SDL_BLENDMODE_BLEND));checked(SDL_SetTextureScaleMode(textures[p],SDL_SCALEMODE_NEAREST));
    }
    texture_owner=renderer;video_revision=~0u;
}
void OriginalGraphics::update_video_board(SDL_Renderer*){
 // Seven retail venues place the video scratch buffer on page 15, which
 // their stand UVs do not reference. Do not overwrite the preserved goal map.
 if(video_offset<0 || (video_page!=8 && video_page!=9) || !sequence->team_play)return;
 if(!replaying)video_board->observe(*sequence);
 if(video_revision==video_board->revision())return;
 auto colors=palette;
 if(video_board->retail_frame())std::memcpy(colors.data(),video_board->retail_palette().data(),768);
 else std::memcpy(colors[240].data(),video_board->colors().data(),48);
 std::array<unsigned char,10240*4> rgba{};
 for(std::size_t i=0;i<10240;++i){const auto index=video_board->pixels()[i];for(int c=0;c<3;++c)rgba[i*4+c]=colors[index][c]*4;rgba[i*4+3]=255;}
 const SDL_Rect rectangle{video_offset%256,video_offset/256,128,80};
 checked(SDL_UpdateTexture(textures[video_page],&rectangle,rgba.data(),128*4));video_revision=video_board->revision();
}
void OriginalGraphics::model(float* p,short* faces,float x,float y,float z,float cosine,float sine,bool mirrored,bool clipped){
    if(p[0]>500 || core->curobj>=core->objects.data()+99 || core->curpol>=core->polygons.data()+800)throw std::runtime_error("Original graphics list capacity exceeded");
    auto* first=core->curpol;
    legacy::obj object{p,faces,x,y,z,cosine,sine};if(clipped)core->addobjnc(&object);else if(p==pitch_points.data())core->addobjyc(&object);else if(mirrored)core->addobjyf(&object);else core->addobjy(&object);
    const bool stadium=retail_stadium && std::any_of(stands.begin(),stands.end(),[&](const auto& m){return p==m.points.data();});
    for(auto* poly=first;poly<core->curpol;++poly)stadium_polygons[poly-core->polygons.data()]=stadium;
    if(clip_stadium && !clipped)for(auto* poly=first;poly<core->curpol;++poly){
        auto& depths=polygon_depths[poly-core->polygons.data()];
        for(int j=0;j<poly->pts[0];++j){
            depths[j]=1/poly->z;
            for(int k=0;k<int(p[0]);++k){const auto* point=core->rotated.data()+k*4;
                if(point[0]>=15 && int(point[1])==poly->pts[2+j*2] && int(point[2])==poly->pts[3+j*2]){depths[j]=1/point[0];break;}
            }
        }
    }
}
void OriginalGraphics::prepare_stadium_occlusion(){
    occlusion.covers.clear();
    const auto& m=core->rot.e;
    for(std::size_t part=0;part<stands.size();++part){
        const auto& mesh=stands[part];const auto& origin=stand_positions[part];std::size_t at=1;
        for(int f=0;f<mesh.faces[0];++f){
            const int n=mesh.faces[at++],material=mesh.faces[at++];const auto indices=at;at+=n;
            if(n<3 || material>=256 || (material<0 && n>4))continue;
            // Cutout textures must not become solid occluders. The tunnel's CD
            // roof, walls and crowd are opaque; leave any cutout face unchanged.
            if(material<0){const int id=material<-2000?-material-2000:-material;const auto& uv=texture_coords.at(id);
                const auto& pixels=stadium_parts[(uv[0]>>24)==8?1:2];
                int left=255,right=0,top=255,bottom=0;
                for(int j=0;j<n;++j){const int u=(uv[n+j]&0xffffff)>>16,v=(uv[j]&0xffffff)>>16;left=std::min(left,u);right=std::max(right,u);top=std::min(top,v);bottom=std::max(bottom,v);}
                bool cutout=false;for(int v=top;v<=bottom && !cutout;++v)for(int u=left;u<=right;++u)if(!pixels[v*256+u]){cutout=true;break;}
                if(cutout)continue;
            }
            OriginalOcclusion::Polygon camera;
            for(int j=0;j<n;++j){const auto* p=mesh.points.data()+1+mesh.faces[indices+j]*3;
                const float x=p[0]+origin[0],y=p[1]+origin[1],z=p[2]+origin[2];
                OriginalOcclusion::Vertex v;v.draw.position={x*m.r00+z*m.r02+m.r03,x*m.r10+y*m.r11+z*m.r12+m.r13};v.depth=x*m.r20+y*m.r21+z*m.r22+m.r23;camera.push_back(v);
            }
            camera=OriginalOcclusion::clip(camera,[](const auto& v){return v.depth-15;},true);
            for(auto& v:camera){const auto p=v.draw.position;const float q=core->q/v.depth;
                v.draw.position={output_view.x+output_view.w/640*float(int(p.x*q+320)),output_view.y+output_view.h-output_view.h/480*float(int(p.y*q+240))};v.depth=1/v.depth;
            }
            for(std::size_t j=1;j+1<camera.size();++j)if(OriginalOcclusion::cross(camera[0].draw.position,camera[j].draw.position,camera[j+1].draw.position)>0)occlusion.add({camera[0],camera[j],camera[j+1]});
        }
    }
}
void OriginalGraphics::flush(SDL_Renderer* r){
    const float sx=output_view.w/640,sy=output_view.h/480;
    SDL_BlendMode previous_blend;checked(SDL_GetRenderDrawBlendMode(r,&previous_blend));checked(SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND));
    std::vector<legacy::pols*> sorted;for(auto* p=core->polygons.data();p<core->curpol;++p)sorted.push_back(p);
    std::stable_sort(sorted.begin(),sorted.end(),[](auto* a,auto* b){return a->z>b->z;});last_polygons+=sorted.size();
    for(auto* poly:sorted){
        const int n=poly->pts[0],col=poly->pts[1];if(n<1 || n>16)throw std::runtime_error("Invalid original polygon");
        if(n==1){
            if(poly->z<5 || (col!=0 && col!=1))continue;
            const float x=output_view.x+sx*float(poly->pts[2]),y=output_view.y+output_view.h-sy*float(poly->pts[3]);
            if(!std::isfinite(x) || !std::isfinite(y))continue;
            const auto c=palette[col?2:31];
            if(!col){checked(SDL_SetRenderDrawColor(r,8,20,15,255));SDL_FRect edge{x-2.5f*sx,y-2.5f*sy,5*sx,5*sy};checked(SDL_RenderFillRect(r,&edge));}
            checked(SDL_SetRenderDrawColor(r,Uint8(c[0]*4),Uint8(c[1]*4),Uint8(c[2]*4),255));
            SDL_FRect dot{x-1.5f*sx,y-1.5f*sy,3*sx,3*sy};checked(SDL_RenderFillRect(r,&dot));continue;
        }
        SDL_Texture* tex=nullptr;const legacy::dword* uv=nullptr;
        SDL_FColor color{1,1,1,1};
        if(col<0){int idx=col<-2000?-col-2000:-col;if(idx<=0 || idx>=core->clip_tex || n>5)throw std::runtime_error("Unsupported original texture");uv=core->textures[idx];const auto page=std::uint32_t(uv[0])>>24;if(page>=textures.size())throw std::runtime_error("Unrestored texture page");tex=textures[page];}
        else if(col==256 || col==257){const float value=col==256?0.0f:192.0f/255; color={value,value,value,.5f};} // Original makefilter targets 0/48 at half strength; SDL omits palette requantization.
        else {if(col>=256)throw std::runtime_error("Unrestored palette filter");auto c=(stadium_polygons[poly-core->polygons.data()]?stadium_palette:palette)[std::size_t(col)];color={float(c[0]*4)/255,float(c[1]*4)/255,float(c[2]*4)/255,1};}
        std::vector<SDL_Vertex> verts;for(int i=0;i<n;++i){SDL_Vertex v{};v.position={output_view.x+sx*float(poly->pts[2+i*2]),output_view.y+output_view.h-sy*float(poly->pts[3+i*2])};v.color=color;
            if(uv)v.tex_coord={float((uv[n+i]&0xffffff)>>16)/256,float((uv[i]&0xffffff)>>16)/256};verts.push_back(v);}
        if(n==2){checked(SDL_SetRenderDrawColorFloat(r,color.r,color.g,color.b,1));checked(SDL_RenderLine(r,verts[0].position.x,verts[0].position.y,verts[1].position.x,verts[1].position.y));}
        else if(clip_stadium && polygon_depths[poly-core->polygons.data()][0]>0){
            const auto& depth=polygon_depths[poly-core->polygons.data()];
            for(int i=1;i<n-1;++i)for(const auto& piece:occlusion.visible({{verts[0],depth[0]},{verts[i],depth[i]},{verts[i+1],depth[i+1]}})){
                std::vector<SDL_Vertex> vertices;for(const auto& v:piece)vertices.push_back(v.draw);
                std::vector<int> indices;for(int j=1;j<int(vertices.size())-1;++j){indices.push_back(0);indices.push_back(j);indices.push_back(j+1);}
                checked(SDL_RenderGeometry(r,tex,vertices.data(),int(vertices.size()),indices.data(),int(indices.size())));
            }
        }
        else {std::vector<int> indices;for(int i=1;i<n-1;++i){indices.push_back(0);indices.push_back(i);indices.push_back(i+1);}checked(SDL_RenderGeometry(r,tex,verts.data(),n,indices.data(),int(indices.size())));}
    }
    stadium_polygons.fill(false);
    if(clip_stadium)for(auto& depths:polygon_depths)depths.fill(0);
    checked(SDL_SetRenderDrawBlendMode(r,previous_blend));
}
short* OriginalGraphics::player_faces(int number,bool mirrored,bool wire,bool alternate_skin){
    if(wire)return recovered::player_fw;
    if(number==1)return mirrored?recovered::player_fg1x:recovered::player_fg1;
    if(number==12)return mirrored?recovered::player_fg2x:recovered::player_fg2;
    if(alternate_skin)return number>11?(mirrored?recovered::player_f2xb:recovered::player_f2b):(mirrored?recovered::player_f1xb:recovered::player_f1b);
    if(number>11)return mirrored?recovered::player_f2x:recovered::player_f2;
    return mirrored?recovered::player_f1x:recovered::player_f1;
}
std::array<short,87> OriginalGraphics::numbered_player_faces(int number,bool mirrored,int shirt,bool alternate_skin,int number_texture_base,bool dark_number){
    if(number<1 || number>22)throw std::out_of_range("Player outside restored roster");
    std::array<short,87> faces;std::copy_n(player_faces(number,mirrored,false,alternate_skin),faces.size(),faces.begin());
    if(faces[0]!=13 || faces[81]!=4)throw std::runtime_error("Unverified shirt-number face layout");
    if(!shirt)shirt=(number-1)%11+1;
    if(shirt<1 || shirt>20)throw std::out_of_range("Shirt outside restored squad");
    if(number_texture_base){
        if(number_texture_base<1 || number_texture_base+40>=2000)throw std::out_of_range("Invalid retail number material range");
        faces[82]=short(-2000-number_texture_base-(dark_number?20:0)-(shirt-1));return faces;
    }
    // Optional retail media absent: the preserved atlas supplies 1-15 only.
    if(shirt>15){faces[0]=12;return faces;}
    faces[82]=short(-2000-(number<=11?recovered::shirt_home:recovered::shirt_away)-(shirt-1));
    return faces;
}
void OriginalGraphics::corner_flags(){
 // Original render3d flag geometry and wind deformation; derive animation from
 // simulation time so pause and replay do not consume gameplay time or RNG.
 auto points=std::to_array(recovered::flag_p);auto faces=std::to_array(recovered::flag_f);
 const auto& e=sequence->engine;
 if(e.wind_on){
  const float speed=std::clamp(e.wind_speed,0.0f,128.0f),fs=1.57079632679f*(speed+16)/144;
  const float fx=8.5f*std::sin(fs)+1,fy=8.5f*std::cos(fs);
  points[25]=fx*e.wind_x;points[26]=15-fy;points[27]=fx*e.wind_y;
  const int tick=replaying?replay_cursor:sequence->ticks;
  const int phase=int(float(tick)/std::max(1.0f,(128-speed)/10))%3;
  faces[32]=faces[37]=short(-2579-phase);
 }else{points[25]=1.751f;points[26]=6.629f;points[27]=1.751f;}
 for(float x:{0.0f,1280.0f})for(float z:{0.0f,-800.0f}){model(points.data(),faces.data(),x,0,z,1,0,false,true);++scene_counts.flags;}
}
void OriginalGraphics::pitch_markings(){
    // Original render3d uses addobjfc: clip ground lines individually instead of
    // dropping an entire mesh when its origin is behind the camera.
    const auto marking=[this](float* points,short* faces,float x,float y,float z){
        legacy::obj object{points,faces,x,y,z,1,0};core->addobjfc(&object);
    };
    marking(recovered::lines_p,recovered::lines_f,0,0,0);
    marking(recovered::circle_pa,recovered::circle_fa,640,0,-400);
    marking(recovered::l1_p1,recovered::l_f1a,210,0,-200);marking(recovered::l2_p1,recovered::l_f1a,1070,0,-200);
    marking(recovered::l3_p1,recovered::l_f1a,1070,0,-600);marking(recovered::l4_p1,recovered::l_f1a,210,0,-600);
    marking(recovered::l5_p1,recovered::l_f3a,640,0,-200);marking(recovered::l6_p1,recovered::l_f3a,640,0,-600);
    marking(recovered::s_circle_p1a,recovered::s_circle_f1a,123,0,-400);marking(recovered::s_circle_p2a,recovered::s_circle_f2a,1157,0,-400);
    for(float x:{128.0f,640.0f,1152.0f})marking(recovered::spot_p,recovered::spot_f,x,0,-400);
    scene_counts.markings=13;
}
void OriginalGraphics::goals(){
    for(int side=0;side<2;++side){const float x=side?1280.0f:0.0f;
        const std::array<std::array<float,3>,4> locations{{{x,0,-443.5f},{x,0,-356.5f},{x,37,-400},{x+(side?28.0f:-28.0f),0,-400}}};
        for(int part=0;part<4;++part){
            const auto& p=locations[part];legacy::obj object{nullptr,nullptr,p[0],p[1],p[2],1,0};
            core->objdepd(&object,recovered::goals[side*4+part]);model(object.points,object.faces,object.x,object.y,object.z,1,0,false,true);++scene_counts.goal_parts;
        }
    }
}
legacy::obj OriginalGraphics::player_object(float* points,short* faces,const original::match_player& p){
    // new_update_coords/realtime_coords negate crot, then render3d negates it again.
    // We start with gameplay directions, so those two negations cancel.
    return {points,faces,p.tm_x,p.tm_z,-p.tm_y,p.tm_xdis,p.tm_ydis};
}
std::array<float,3> OriginalGraphics::captured_ball(const legacy::obj& o,bool mirrored){
    // Same object transform as render3d's "Motion captured ball" block.
    const float y=-o.points[70],z=mirrored?-o.points[72]:o.points[72];
    return {-y*o.crot+z*o.srot+o.x,o.points[71]+o.y,y*o.srot+z*o.crot+o.z};
}
void OriginalGraphics::step_camera(){
    if(!sequence->team_play)return;
    auto& e=replaying?*replay_engine:sequence->engine;int mode=match_camera==0?0:match_camera==1?14:match_camera==2?6:match_camera==3?1:2;
    // Original close goal-kick views, selected only while that restart is active.
    if(match_camera==0 && e.match_mode>=7 && e.match_mode<=10)mode=e.match_mode<=8?9:10;
    if(match_camera==0 && e.match_mode>=1 && e.match_mode<=4)mode=e.match_mode<=2?11:12;
    if(match_camera==0 && (e.match_mode==11 || e.match_mode==12))mode=13;
    if(e.just_scored>0 && e.just_scored<original::action_constants::ORIGINAL_SCORE_WAIT-30 && e.goal_scorer>0)mode=15;
    if(camera_generation!=sequence->generation){camera_motion={};camera_current.tx=e.ballx;camera_current.ty=e.bally;}
    auto target=original_camera(mode,e,e.ballx,e.bally,e.ballz,camera_current,&camera_motion);
    if(mode!=1 && mode!=2){
    target.x=std::clamp(target.x,-stadium_length/2.0f,1280+stadium_length/2.0f);
    target.y=std::clamp(target.y,-stadium_width/2.0f,800+stadium_width/2.0f);
    target.z=std::min(target.z,float(stadium_height));
    }
    camera_previous=camera_current;
    if(camera_generation!=sequence->generation){camera_current=camera_previous=target;camera_generation=sequence->generation;return;}
    if(mode==1 || mode==2){camera_current=target;return;} // These source views supply their own inertia.
    // Fixed 20 Hz timing replaces the original DOS updates/game_speed timer.
    const auto ease=[](float from,float to,float rate){return from+(to-from)*rate;};
    camera_current={ease(camera_current.x,target.x,.2f),ease(camera_current.y,target.y,.2f),ease(camera_current.z,target.z,.2f),
        ease(camera_current.tx,target.tx,.3f),ease(camera_current.ty,target.ty,.3f),ease(camera_current.tz,target.tz,.3f),ease(camera_current.scale,target.scale,.2f)};
}
void OriginalGraphics::draw(SDL_Renderer* r,SDL_FRect viewport){
    output_view=viewport;
    clip_stadium=false;
    auto& presentation=walkoff.active()?walkoff.presentation:prematch.active()?prematch.presentation:exit.active()?exit.presentation:replaying?replay_presentation:this->presentation;
    last_polygons=0;scene_counts={};environment->ground_pixels=environment->sky_pixels=0;load_textures(r);update_video_board(r);update_tracker();
    SDL_Rect cliprect{int(viewport.x),int(viewport.y),int(viewport.w),int(viewport.h)};checked(SDL_SetRenderClipRect(r,&cliprect));
    checked(SDL_SetRenderDrawColor(r,32,72,43,255));SDL_FRect bg=viewport;checked(SDL_RenderFillRect(r,&bg));
    if(walkoff.active()){const auto c=walkoff.camera();core->camera(c.x,c.z,c.y,c.tx,c.tz,c.ty,c.scale,640,480);}
    else if(prematch.active()){const auto c=prematch.camera();core->camera(c.x,c.z,c.y,c.tx,c.tz,c.ty,c.scale,640,480);}
    else if(exit.active())core->camera(tunnel_x,750,250,tunnel_x,0,850,560,640,480);
    else if(overview)core->camera(640+std::cos(angle)*2800,1700,400+std::sin(angle)*2800,640,0,400,650,640,480);
    else if(action_view && sequence->team_play){
        if(camera_generation!=sequence->generation)step_camera();
        const float alpha=sequence->running?presentation_alpha():1;
        const auto lerp=[&](float a,float b){return a+(b-a)*alpha;};
        core->camera(lerp(camera_previous.x,camera_current.x),lerp(camera_previous.z,camera_current.z),lerp(camera_previous.y,camera_current.y),
            lerp(camera_previous.tx,camera_current.tx),lerp(camera_previous.tz,camera_current.tz),lerp(camera_previous.ty,camera_current.ty),
            lerp(camera_previous.scale,camera_current.scale),640,480);
    }
    else if(action_view && sequence->play){
        presentation.observe(sequence->engine,sequence->generation,sequence->ticks);
        const float alpha=sequence->running?presentation_alpha():1;
        const auto a=presentation.player(sequence->engine,1,alpha).state;
        const auto b=presentation.player(sequence->engine,2,alpha).state;
        const float x=std::clamp((a.tm_x+b.tm_x)/2+80,220.0f,1060.0f),y=std::clamp((a.tm_y+b.tm_y)/2,140.0f,660.0f);
        core->camera(x,260,y+410,x,8,y,560,640,480);
    }
    else if(action_view && sequence->goal_review){
        const float target=sequence->review_left?15.0f:1265.0f;
        const float direction=sequence->review_left?1.0f:-1.0f;
        core->camera(target+direction*std::cos(angle+.65f)*180,95,400+std::sin(angle+.65f)*180,target,15,400,560,640,480);
    }
    else if(action_view && sequence->attack)core->camera(1000+std::cos(angle)*570,320,400+std::sin(angle)*570,1000,15,400,560,640,480);
    else if(action_view)core->camera(640+std::cos(angle)*230,130,400+std::sin(angle)*230,640,8,400,560,640,480);
    else core->camera(640+std::cos(angle)*90,40,400+std::sin(angle)*90,640,12,400,560,640,480);
    if(overview || action_view){
        model(pitch_points.data(),recovered::pitch_f,0,0,0);flush(r);core->clear();
        draw_environment(r);pitch_markings();corner_flags();flush(r);core->clear();
    }
    // A fixed background pass lets actors overpaint the tunnel roof/crowd.
    // During period-end exits, sort the original stand faces with actors;
    // ground shadows stay below the stand floors, as they do below the pitch.
    const bool tunnel_depth=action_view && exit.active();
    const auto stadium_models=[&]{
        for(std::size_t i=0;i<4;++i){auto& m=stands[i];const auto& t=stand_positions[i];model(m.points.data(),m.faces.data(),t[0],t[1],t[2],1,0,false,true);++scene_counts.stands;}
    };
    if((overview || (action_view && sequence->team_play && match_stadium)) && !tunnel_depth){
        stadium_models();
        flush(r);core->clear();
    }
    if(action_view){
        if(walkoff.active() && match_stadium){prepare_stadium_occlusion();clip_stadium=true;}
        auto& e=walkoff.active()?*walkoff.engine:prematch.active()?*prematch.engine:exit.active()?*exit.engine:replaying?*replay_engine:sequence->engine;
        if(!exit.active() && !prematch.active() && !walkoff.active())presentation.observe(e,replaying?1:sequence->generation,replaying?replay_cursor:sequence->ticks);
        const float alpha=walkoff.active()?walkoff.alpha():prematch.active()?prematch.alpha():exit.active()?exit.alpha():sequence->running?presentation_alpha():1.0f;
        const auto visual_ball=presentation.displayed_ball(e,alpha);
        std::array<OriginalPresentation::Player,22> players{};
        // Ground pass precedes the players and goals, following render3d's shadow layer.
        for(int i:sequence->visible_players){
            if(!e.teams[i].guy_on || (sequence->team_play && !e.teams[i].draw_me))continue;
            players[i]=presentation.player(e,i,alpha);const auto& p=players[i].state;
            model(recovered::plshad_p,recovered::plshad_f,p.tm_x,0,-p.tm_y);++scene_counts.player_shadows;
            if(!exit.active() && !prematch.active() && !walkoff.active() && sequence->play && !sequence->cpu_vs_cpu && !sequence->penalty_wait && (i==sequence->selected_for(0) || (sequence->head_to_head && i==sequence->selected_for(1))))model(recovered::plhi_p,sequence->head_to_head && i==sequence->selected_for(1)?recovered::plhi1_f2:recovered::plhi1_f1,p.tm_x,0,-p.tm_y);
        }
        if(!exit.active()){model(recovered::ballshad_p,recovered::ballshad_f,visual_ball.x,0,-visual_ball.y);++scene_counts.ball_shadows;}
        add_tracker_sprites(true);flush(r);core->clear();
        if(tunnel_depth && match_stadium)stadium_models();
        goals();
        for(int i:sequence->visible_players){
            if(!e.teams[i].guy_on || (sequence->team_play && !e.teams[i].draw_me))continue;
            auto& visual=players[i];const auto& p=visual.state;
            const int material_player=(sequence->full_match && (e.match_half&1))?(p.tm_player<=11?p.tm_player+11:p.tm_player-11):p.tm_player;
            const int country=material_player<=11?sequence->home_team:sequence->away_team;
            const int retail_appearance=euro_kits.appearance(country,p.shirt);
            auto faces=numbered_player_faces(material_player,visual.mirrored,sequence->national_teams?p.shirt:0,sequence->national_teams && (retail_appearance>=0?retail_appearance==1:cd_kits.alternate_skin(country,p.shirt)),number_texture_base,(material_player!=1 && material_player!=12) && dark_numbers[material_player>11]);
            auto object=player_object(visual.pose.data(),wire?recovered::player_fw:faces.data(),p);
            model(object.points,object.faces,object.x,object.y,object.z,object.crot,object.srot,visual.mirrored);
            if(p.tm_player<=11)++scene_counts.home_players;else ++scene_counts.away_players;
            if(p.tm_player==1 || p.tm_player==12)++scene_counts.keepers;
        }
        if(sequence->full_match)for(int i=0;i<3;++i){
            if(e.refs[i].fstep<=0)continue;
            auto visual=presentation.player(e,22+i,alpha);auto& p=visual.state;
            auto object=player_object(visual.pose.data(),i?recovered::player_fl:recovered::player_fr,p);
            model(object.points,object.faces,object.x,object.y,object.z,object.crot,object.srot,visual.mirrored);++scene_counts.officials;
        }
        const auto rotation=presentation.ball_rotation(alpha);
        legacy::obj ball{recovered::footy_p,recovered::footy_f,visual_ball.x,visual_ball.z,-visual_ball.y,rotation[0],rotation[1]};
        if(!exit.active()){core->addobjxy(&ball,rotation[2],rotation[3]);++scene_counts.rotating_balls;}
    }else{
        auto& f=clips[std::size_t(clip)].frames[std::size_t(frame)];
        model(recovered::plshad_p,recovered::plshad_f,640,0,-400);++scene_counts.player_shadows;
        if(f[71]>=0){model(recovered::ballshad_p,recovered::ballshad_f,640-f[70],0,-400-f[72]);++scene_counts.ball_shadows;}
        flush(r);core->clear();if(overview)goals();
        auto faces=numbered_player_faces(2,false);
        model(f.data(),wire?recovered::player_fw:faces.data(),640,0,-400,-1,0);
        if(f[71]>=0)model(recovered::footy_p,recovered::footy_f,640-f[70],f[71],-400-f[72]);
    }
    add_tracker_sprites(false);flush(r);draw_overlay(r);checked(SDL_SetRenderClipRect(r,nullptr));
}
void OriginalGraphics::draw_environment(SDL_Renderer* r){
    environment->render(*core,palette);
    if(environment_owner!=r || !environment_texture){
        SDL_DestroyTexture(environment_texture);environment_texture=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,640,480);
        if(!environment_texture)throw std::runtime_error(SDL_GetError());environment_owner=r;
        checked(SDL_SetTextureBlendMode(environment_texture,SDL_BLENDMODE_BLEND));checked(SDL_SetTextureScaleMode(environment_texture,SDL_SCALEMODE_NEAREST));
    }
    checked(SDL_UpdateTexture(environment_texture,nullptr,environment->pixels().data(),640*4));checked(SDL_RenderTexture(r,environment_texture,nullptr,&output_view));
}
bool OriginalGraphics::clean_match_view()const{
 if(exit.active() || prematch.active() || walkoff.active())return true;
 const auto& s=*sequence;
 if(menu_ui && action_view && paused && s.full_match && !s.full_time && !s.interval_ticks)return true;
 if(action_view && replaying)return !match_panels;
 return action_view && s.team_play && !match_panels && !paused && !s.full_time && !s.interval_ticks &&
     !s.human_restart() && !s.penalty_human_keeper() && !(s.penalty_human_taker() && s.engine.match_mode);
}
void OriginalGraphics::draw_overlay(SDL_Renderer* r){
 radar_players=0;const auto& s=*sequence;if(!action_view || !s.team_play || exit.active() || prematch.active() || walkoff.active())return;
 const auto& e=replaying?*replay_engine:s.engine;const auto& view=replaying?replay_presentation:presentation;
 const float alpha=presentation_alpha();overlay->clear(palette);
 if(show_radar){
  std::vector<OriginalOverlay::Marker> markers;
  for(int i:s.visible_players)if(e.teams[i].guy_on && e.teams[i].draw_me){const auto p=view.player(e,i,alpha);markers.push_back({p.state.tm_x,p.state.tm_y,i});}
  const auto b=view.displayed_ball(e,alpha);overlay->scanner(markers,b.x,b.y,e.match_half);radar_players=int(markers.size());
 }
 char time[32];std::snprintf(time,sizeof(time),"%d:%02d",e.match_time.min,int(e.match_time.sec));
 const auto team=[&](bool away){std::string name=s.national_teams?original_teams[away?s.away_team:s.home_team].name:s.team_label(away);if(name.size()>11)name=name.substr(0,10)+".";return name;};
 overlay->score(team(false),team(true),s.shootout?s.penalty_scores[0]:s.blue_goals,s.shootout?s.penalty_scores[1]:s.red_goals,replaying?"REPLAY":s.shootout?"PENALTIES":time,s.pending_subs[0].slot>=0,s.pending_subs[1].slot>=0);
 if(!replaying && s.match_notice.ticks>0)overlay->notice(s.match_notice.title,s.match_notice.detail,s.match_notice.sprite);
 if(overlay_owner!=r || !overlay_texture){
  SDL_DestroyTexture(overlay_texture);overlay_texture=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,640,480);
  if(!overlay_texture)throw std::runtime_error(SDL_GetError());overlay_owner=r;
  checked(SDL_SetTextureBlendMode(overlay_texture,SDL_BLENDMODE_BLEND_PREMULTIPLIED));checked(SDL_SetTextureScaleMode(overlay_texture,SDL_SCALEMODE_LINEAR));
 }
 // Filter HUD lettering independently of the pitch/player textures. Correct
 // alpha keeps pale text bright and translucent panels at their original opacity.
 overlay_upload.resize(640*480);
 checked(SDL_PremultiplyAlpha(640,480,SDL_PIXELFORMAT_RGBA32,overlay->pixels().data(),640*4,SDL_PIXELFORMAT_RGBA32,overlay_upload.data(),640*4,false));
 checked(SDL_UpdateTexture(overlay_texture,nullptr,overlay_upload.data(),640*4));checked(SDL_RenderTexture(r,overlay_texture,nullptr,&output_view));
}

}
