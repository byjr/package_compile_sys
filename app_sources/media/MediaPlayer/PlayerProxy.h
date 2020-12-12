class PlayerProxy{
    bool set_next_uri(const std::string uri);

    bool set_uri(const std::string uri,
                 output_update_meta_cb_t meta_cb);

    int play(output_transition_cb_t callback);

    int stop(void);

    int pause(void);

    int seek(long position_nanos);

    int get_position(long* track_duration,
                     long* track_pos);

    int get_volume(float* v);

    int set_volume(float value);

    int get_mute(int* m);

    int set_mute(int m);

    int get_state(void);

    int resume(void);
};