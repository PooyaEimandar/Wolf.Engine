//! Manage game time

#[derive(Default)]
pub struct w_gametime_t
{
    last_time: f64,
    max_delta: f64,

    //derived timing data uses a canonical tick format.
    elapsed_ticks: u64,
    total_ticks: u64,
    left_over_ticks: u64,

    //members for tracking the framerate.
    frame_count: u32,
    fps: u32,
    frames_this_second: u32,
    seconds_counter: f64,

    //members for configuring fixed timestep mode.
    fixed_time_step: bool,
    target_elapsed_ticks: u64
}

#[repr(C)]
pub struct w_gametime
{
    pub ptr: w_gametime_t
}
