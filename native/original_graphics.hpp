#pragma once
#include "graphics_types.hpp"
#include "archive.hpp"
#include "original_environment.hpp"
#include "original_video_board.hpp"
#include "original_overlay.hpp"
#include "original_motion.hpp"
#include "original_presentation.hpp"
#include "original_exit.hpp"
#include "original_prematch.hpp"
#include "original_walkoff.hpp"
#include "original_occlusion.hpp"
#include "player_original.hpp"
#include "original_camera.hpp"
#include "saved_replay.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <deque>
namespace actua::legacy {
// Windows-owned storage; the methods below are extracted unchanged from 3DENG.C.
class Core {
public:
    qmat rot{},tran{};
    float eye_x=0,eye_y=0,eye_z=0;
    float q=500,qa=10,persc=500.0f/15,scrdr=1.0f/15;
    buff_info_ext render_buff{};
    std::array<float,2000> rotated{};
    std::array<pols,1000> polygons{};
    std::array<scrpt,20000> points{};
    std::array<objs,256> objects{};
    float* rotlist=rotated.data();pols* curpol=polygons.data();scrpt* curpts=points.data();objs* curobj=objects.data();
    void addobjsp(datapt*,word*);
    void matcop(mat,mat);void matmul(mat,mat);
    texture textures[2000]{}; int clip_base=0,clip_tex=0;
    void addobjnc(obj*);
    void addpolyc(face,word,word);void addlinec(face,word);void addpolsc(obj*);void addobjyc(obj*);void addobjfc(obj*);
    void addpoly(face,word,word);void addline(face,word);
    void add3dcmap(face,word);void add3demap(face,word);void addpols(obj*);void addobjy(obj*);void addobjyf(obj*);
    void addobjxy(obj*,floatpt,floatpt);
    float objdepd(obj*,det*);
    void camera(float x,float y,float z,float tx,float ty,float tz,float distance,int width,int height);
    void clear(){clip_tex=clip_base;curpol=polygons.data();curpts=points.data();curobj=objects.data();}
};
}
namespace actua {

class OriginalActions;
class OriginalMenuArt;
class OriginalGraphics {
public:
    explicit OriginalGraphics(const std::filesystem::path& data);
    ~OriginalGraphics();
    OriginalGraphics(const OriginalGraphics&)=delete;
    OriginalGraphics& operator=(const OriginalGraphics&)=delete;
    void load_cd_graphics(const std::filesystem::path& root);
    int next_stadium=0;
    int stadium()const{return stadium_index;}
    static constexpr std::array<const char*,13> stadium_names{"England (legacy)","Scotland (legacy)","Spain (legacy)","Italy (legacy)","Brazil (legacy)","Wembley","Villa Park","Elland Road","St James Park","Old Trafford","Anfield","Hillsborough","City Ground"};
    void change_stadium(int delta);
    void apply_stadium();
    bool stadium_available(int index)const;
    int stadium_event()const{return video_board->event();}
    bool has_cd_stadium()const{return !stadium_parts[0].empty();}
    bool has_original_kit(int team)const;
    std::array<unsigned char,3> palette_color(int index)const{return palette.at(index);}
    unsigned ground_pixels()const{return environment->ground_pixels;}
    unsigned sky_pixels()const{return environment->sky_pixels;}
    std::vector<OriginalClip> clips;
    std::unique_ptr<OriginalActions> sequence;
    std::shared_ptr<OriginalMenuArt> menu_ui;
    OriginalPresentation presentation;
    OriginalExit exit;
    OriginalPrematch prematch;
    OriginalWalkoff walkoff;
    bool walkoffs_enabled=true;
    bool start_walkoff();
    void finish_walkoff();
    bool prematches_enabled=true,opening_input_guard=false;
    void start_prematch();
    void finish_prematch();
    void advance_prematch(double seconds);
    bool exits_enabled=true;
    void start_exit();
    void advance_exit(double seconds){if(!exit.paused){exit.advance(seconds);video_board->advance(seconds);}}
    float presentation_alpha()const{return replaying && paused?1.0f:float(clock);}
    bool action_view=false;
    int match_camera=0;
    bool match_stadium=true;
    const char* camera_name()const{return match_camera==0?"Original auto":match_camera==1?"Isometric":match_camera==2?"Follow ball":match_camera==3?"Plan":"Goal line";}
    void change_camera(int delta=1){const int count=replaying?5:3;match_camera=(match_camera+delta+count)%count;camera_motion={};camera_generation=~std::uint64_t(0);}
    CameraPose camera_pose()const{return walkoff.active()?walkoff.camera():prematch.active()?prematch.camera():camera_current;}
    int clip=40,frame=0,kit=0;bool paused=false,wire=false,overview=false;
    float angle=0.4f;
    void advance(double seconds);
    bool replaying=false;
    bool start_replay(bool automatic=false);
    void finish_replay();
    std::filesystem::path save_replay(const std::filesystem::path& directory);
    void open_saved_replay(const std::filesystem::path& path);
    bool saved_replay_active()const{return saved_return!=nullptr;}
    std::string replay_message;
    std::size_t replay_frames()const{return history.size();}
    int replay_position()const{return replay_cursor;}
    int replay_speed()const{return replay_rate;}
    bool replay_looping()const{return replay_loop;}
    void seek_replay(int frame);
    void step_replay(int delta){seek_replay(replay_cursor+delta);}
    void change_replay_speed(int delta);
    void toggle_replay_pause();
    void toggle_replay_loop(){if(replaying){replay_loop=!replay_loop;replay_automatic=false;}}
    void change_clip(int delta);
    void change_kit(int delta);
    void draw(SDL_Renderer* renderer,SDL_FRect viewport={24,116,640,480});
    // Zero preserves the original projection; 720/1080 enable aspect-aware rendering.
    int presentation_height=0;
    bool show_radar=true,match_panels=false;
    bool clean_match_view()const;
    bool has_euro96_kits()const{return euro_kits.loaded();}
    int trophy_film(int home,int away,int champion)const{return euro_kits.selection(home,away,champion==away);}
    bool has_retail_numbers()const{return shirt_numbers.loaded();}
    bool has_retail_players()const{return euro_players.loaded();}
    int radar_players=0;
    static legacy::obj player_object(float* points,short* faces,const original::match_player& player);
    static std::array<float,3> captured_ball(const legacy::obj& object,bool mirrored);
    std::size_t polygon_count()const{return last_polygons;}
    static short* player_faces(int player_number,bool mirrored,bool wire,bool alternate_skin=false);
    static std::array<short,87> numbered_player_faces(int player_number,bool mirrored,int shirt=0,bool alternate_skin=false,int number_texture_base=0,bool dark_number=false);
    struct SceneCounts {int flags=0,officials=0,stands=0,markings=0,goal_parts=0,player_shadows=0,ball_shadows=0,rotating_balls=0,home_players=0,away_players=0,keepers=0;};
    SceneCounts scene_counts{};
    bool tracker_active()const;
    void update_tracker();
    const std::vector<original::PlayerEngine::PitchSprite>& tracker_points()const{return tracker_points_;}
    int tracker_markers=0;

private:
    std::unique_ptr<original::PlayerEngine> tracker_engine;
    std::vector<original::PlayerEngine::PitchSprite> tracker_points_;
    std::uint64_t tracker_generation=~std::uint64_t(0);
    int tracker_tick=-1;
    void add_tracker_sprites(bool shadows);
    struct ReplayReturn {
        std::unique_ptr<OriginalActions> sequence;
        std::deque<ReplayFrame> history;
        int recorded_tick,replayed_goals,stadium,next_stadium,camera;
        std::uint64_t history_generation,camera_generation,presentation_generation;
        bool action_view,overview,match_panels,paused;
        double clock;
        CameraMotion motion;
        CameraPose previous,current;
        std::shared_ptr<OriginalMenuArt> menu_ui;
    OriginalPresentation presentation;
        OriginalVideoBoard board;
        explicit ReplayReturn(const OriginalVideoBoard& b):board(b){}
    };
    std::unique_ptr<ReplayReturn> saved_return;
    void restore_replay_scene();
    std::deque<ReplayFrame> history;
    std::unique_ptr<original::PlayerEngine> replay_engine;
    OriginalPresentation replay_presentation;
    int replay_cursor=0,recorded_tick=-1,replayed_goals=0;
    std::uint64_t history_generation=0;
    bool replay_was_paused=false;
    int replay_saved_camera=0;
    int replay_rate=5;
    bool replay_loop=false,replay_automatic=false;
    CameraMotion camera_motion{};
    void record_replay();
    void load_replay_frame();
    void step_camera();
    CameraPose camera_previous{},camera_current{};
    std::uint64_t camera_generation=~std::uint64_t(0);
    int texture_home=-1,texture_away=-1;
    Archive archive;
    OriginalCdKits cd_kits;
    OriginalEuroKits euro_kits;
    OriginalShirtNumbers shirt_numbers;
    OriginalEuroPlayers euro_players;
    bool retail_player_textures=false;
    int number_texture_base=0;
    std::array<bool,2> dark_numbers{};
    void bind_media_textures();
    std::unique_ptr<OriginalVideoBoard> video_board;
    unsigned video_revision=~0u;
    int video_offset=-1;
    void update_video_board(SDL_Renderer*);
    std::unique_ptr<OriginalEnvironment> environment;
    SDL_Texture* environment_texture=nullptr;
    SDL_Renderer* environment_owner=nullptr;
    int environment_width=0,environment_height=0;
    void draw_environment(SDL_Renderer*);
    std::unique_ptr<OriginalOverlay> overlay;
    SDL_Texture* overlay_texture=nullptr;
    std::vector<std::uint32_t> overlay_upload;
    SDL_Renderer* overlay_owner=nullptr;
    SDL_FRect output_view{24,116,640,480};
    void draw_overlay(SDL_Renderer*);
    std::unique_ptr<legacy::Core> core;
    std::array<SDL_Texture*,17> textures{};
    SDL_Renderer* texture_owner=nullptr;
    std::array<std::array<unsigned char,3>,256> palette{};
    std::vector<std::array<std::uint32_t,8>> texture_coords;
    struct StandMesh {std::vector<float> points;std::vector<short> faces;};
    std::array<StandMesh,4> stands;
    std::array<std::array<float,3>,4> stand_positions{};
    std::array<Bytes,15> stadium_parts;
    bool retail_stadium=false;
    MatchPalette stadium_palette{};
    std::array<bool,1000> stadium_polygons{};
    int video_page=8;
    std::vector<float> pitch_points;
    int stadium_width=0,stadium_length=0,stadium_height=0;
    float tunnel_x=0,tunnel_y=0;
    std::filesystem::path media_root;
    int stadium_index=0;
    void load_cd_stadium(const std::filesystem::path& root,int index=0);
    void corner_flags();
    std::uint64_t presentation_generation=0;
    std::uint64_t walkoff_generation=0;
    double clock=0;std::size_t last_polygons=0;
    void load_textures(SDL_Renderer* renderer);
    void model(float* points,short* faces,float x,float y,float z,float cosine=1,float sine=0,bool mirrored=false,bool clipped=false);
    void flush(SDL_Renderer* renderer);
    OriginalOcclusion occlusion;
    bool clip_stadium=false;
    std::array<std::array<float,16>,1000> polygon_depths{};
    void prepare_stadium_occlusion();
    void pitch_markings();
    void goals();
};
}
