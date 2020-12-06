//! A w_chrono based on Instant

extern crate libc;
use super::w_timespec::*;
use std::mem;
use std::os::raw::c_void;
use std::time::{Duration, Instant};

/// w_chrono_t
#[repr(C)]
pub struct w_chrono_t {
    ptr: *mut c_void,
}
/// Drop implementation w_chrono_t
impl Drop for w_chrono_t {
    fn drop(&mut self) {
        if !self.ptr.is_null() {
            //box will drop instant
            let _box = unsafe { Box::from_raw(self.ptr as *mut Instant) };
            #[cfg(debug_assertions)]
            println!("Instant dropped");
        }
        #[cfg(debug_assertions)]
        println!("w_chrono_t dropped");
    }
}

/// Convert raw pointer of w_chrono_t to Option<&Instant>
/// # Arguments
///
/// * `p` - pointer to chrono object
fn _chrono_to_instant(p: *mut w_chrono_t) -> Option<&'static Instant> {
    unsafe {
        if p.is_null() {
            None
        } else {
            let _ref = p.as_ref();
            match _ref {
                Some(x) => {
                    if !x.ptr.is_null() {
                        Some((x.ptr as *mut Instant).as_ref().unwrap())
                    } else {
                        None
                    }
                }
                None => None,
            }
        }
    }
}

/// Returns duration between two chronos
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
fn _duration_between_chronos(p1: *mut w_chrono_t, p2: *mut w_chrono_t) -> Option<Duration> {
    let _ins_1 = _chrono_to_instant(p1);
    let _ins_2 = _chrono_to_instant(p2);
    match (_ins_1, _ins_2) {
        (Some(a), Some(b)) => a.checked_duration_since(b.clone()),
        _ => None,
    }
}

//     /**
//      * get current timespec
//      * @param pClockType could be on of the following:
//      * <PRE>
//      *
//      * </PRE>
//      * @return a pointer to w_timespan object
//      */
//     W_SYSTEM_EXPORT
//         w_timespec w_chrono_clock_now(_In_ int pClockType);

/// Get C timespec
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
//      CLOCK_REALTIME : represents the machine's best-guess as to the current wall-clock, time-of-day time. it can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.
//      CLOCK_PROCESS_CPUTIME_ID : used for measuring the amount of CPU time consumed by the thread.
#[no_mangle]
pub extern "C" fn w_chrono_clock_now(p_clock_type: u32) -> *mut w_timespec_t {
    let mut _tp = unsafe { mem::zeroed() };
    unsafe { libc::clock_gettime(p_clock_type, &mut _tp) };
    Box::into_raw(Box::new(w_timespec_t {
        secs: _tp.tv_sec as u64,
        nano_seconds: _tp.tv_nsec as u32,
        overflowed: false,
    }))
}

/// Get C timespec in total seconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
//      CLOCK_REALTIME : represents the machine's best-guess as to the current wall-clock, time-of-day time. it can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.
//      CLOCK_PROCESS_CPUTIME_ID : used for measuring the amount of CPU time consumed by the thread.
#[no_mangle]
pub extern "C" fn w_chrono_clock_now_secs(p_clock_type: u32) -> f64 {
    let mut _tp = unsafe { mem::zeroed() };
    unsafe { libc::clock_gettime(p_clock_type, &mut _tp) };

    return (_tp.tv_sec as f64) + ((_tp.tv_nsec as f64) / 1000000000.000f64);
}

/// Get C timespec in total milliseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
//      CLOCK_REALTIME : represents the machine's best-guess as to the current wall-clock, time-of-day time. it can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.
//      CLOCK_PROCESS_CPUTIME_ID : used for measuring the amount of CPU time consumed by the thread.
#[no_mangle]
pub extern "C" fn w_chrono_clock_now_millis(p_clock_type: u32) -> f64 {
    let mut _tp = unsafe { mem::zeroed() };
    unsafe { libc::clock_gettime(p_clock_type, &mut _tp) };

    return (_tp.tv_sec as f64 * 1000.000f64) + ((_tp.tv_nsec as f64) / 1000000.000f64);
}

/// Get C timespec in total microseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
//      CLOCK_REALTIME : represents the machine's best-guess as to the current wall-clock, time-of-day time. it can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.
//      CLOCK_PROCESS_CPUTIME_ID : used for measuring the amount of CPU time consumed by the thread.
#[no_mangle]
pub extern "C" fn w_chrono_clock_now_micros(p_clock_type: u32) -> f64 {
    let mut _tp = unsafe { mem::zeroed() };
    unsafe { libc::clock_gettime(p_clock_type, &mut _tp) };

    return (_tp.tv_sec as f64 * 1000000.000f64) + ((_tp.tv_nsec as f64) / 1000.000f64);
}

/// Get C timespec in total nanoseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
//      CLOCK_REALTIME : represents the machine's best-guess as to the current wall-clock, time-of-day time. it can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.
//      CLOCK_PROCESS_CPUTIME_ID : used for measuring the amount of CPU time consumed by the thread.
#[no_mangle]
pub extern "C" fn w_chrono_clock_now_nanos(p_clock_type: u32) -> f64 {
    let mut _tp = unsafe { mem::zeroed() };
    unsafe { libc::clock_gettime(p_clock_type, &mut _tp) };

    return (_tp.tv_sec as f64 * 1000000000.000f64) + (_tp.tv_nsec as f64);
}

/// Initialize and returns instant chrono object
#[no_mangle]
pub extern "C" fn w_chrono_init_now() -> *mut w_chrono_t {
    let _boxed_instant = Box::new(Instant::now());
    Box::into_raw(Box::new(w_chrono_t {
        ptr: Box::into_raw(_boxed_instant) as *mut c_void,
    }))
}

/// Release instant chrono object
/// # Arguments
///
/// * `p` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_fini(ptr: &mut *mut w_chrono_t) {
    if ptr.is_null() {
        return;
    }
    let _box = unsafe { Box::from_raw(*ptr as *mut w_chrono_t) };
    drop(_box);
    *ptr = ::std::ptr::null_mut();
}

/// Returns elapsed timespec since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_elapsed(p: *mut w_chrono_t) -> *mut w_timespec_t {
    let _instant = _chrono_to_instant(p);
    match _instant {
        Some(x) => {
            let _d = x.elapsed();
            Box::into_raw(Box::new(w_timespec_t {
                secs: _d.as_secs(),
                nano_seconds: _d.subsec_nanos(),
                overflowed: false,
            }))
        }
        None => Box::into_raw(Box::new(w_timespec_t {
            ..Default::default()
        })),
    }
}

/// Returns whole elapsed time in milliseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_elapsed_millis(p: *mut w_chrono_t) -> w_u128 {
    let _instant = _chrono_to_instant(p);
    match _instant {
        Some(x) => w_u128::new(x.elapsed().as_millis()),
        None => w_u128 {
            ..Default::default()
        },
    }
}

/// Returns whole elapsed time in microseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_elapsed_micros(p: *mut w_chrono_t) -> w_u128 {
    let _instant = _chrono_to_instant(p);
    match _instant {
        Some(x) => w_u128::new(x.elapsed().as_micros()),
        None => w_u128 {
            ..Default::default()
        },
    }
}

/// Returns whole elapsed time in nanoseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_elapsed_nanos(p: *mut w_chrono_t) -> w_u128 {
    let _instant = _chrono_to_instant(p);
    match _instant {
        Some(x) => w_u128::new(x.elapsed().as_nanos()),
        None => w_u128 {
            ..Default::default()
        },
    }
}

/// Returns duration between two chrono times
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_duration(p1: *mut w_chrono_t, p2: *mut w_chrono_t) -> *mut w_timespec_t {
    let _d = _duration_between_chronos(p1, p2);
    match _d {
        Some(x) => Box::into_raw(Box::new(w_timespec_t {
            secs: x.as_secs(),
            nano_seconds: x.subsec_nanos(),
            overflowed: false,
        })),
        None => Box::into_raw(Box::new(w_timespec_t {
            ..Default::default()
        })),
    }
}

/// Returns duration between two chrono times in total milliseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_duration_millis(p1: *mut w_chrono_t, p2: *mut w_chrono_t) -> w_u128 {
    let _d = _duration_between_chronos(p1, p2);
    match _d {
        Some(x) => w_u128::new(x.as_millis()),
        None => w_u128 {
            ..Default::default()
        },
    }
}

/// Returns duration between two chrono times in total microseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_duration_micros(p1: *mut w_chrono_t, p2: *mut w_chrono_t) -> w_u128 {
    let _d = _duration_between_chronos(p1, p2);
    match _d {
        Some(x) => w_u128::new(x.as_micros()),
        None => w_u128 {
            ..Default::default()
        },
    }
}

/// Returns duration between two chrono times in total nanoseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
#[no_mangle]
pub extern "C" fn w_chrono_duration_nanos(p1: *mut w_chrono_t, p2: *mut w_chrono_t) -> w_u128 {
    let _d = _duration_between_chronos(p1, p2);
    match _d {
        Some(x) => w_u128::new(x.as_nanos()),
        None => w_u128 {
            ..Default::default()
        },
    }
}
