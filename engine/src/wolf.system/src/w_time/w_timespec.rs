//! A w_timespec same as the C timespec
use std::time::Duration;

/// w_timespec_t for storing time point
#[repr(C)]
#[derive(Default)]
pub struct w_timespec_t {
    /// total seconds
    pub secs: u64,
    /// the fractional part in nanoseconds.
    pub nano_seconds: u32,
    /// true means overflowed
    pub overflowed: bool,
}

/// Drop implementation w_timespec_t
#[cfg(debug_assertions)]
impl Drop for w_timespec_t {
    fn drop(&mut self) {
        println!("dropping w_timespec_t");
    }
}

impl w_timespec_t {
    pub fn from_zero() -> w_timespec_t {
        w_timespec_t {
            secs: 0,
            nano_seconds: 0,
            overflowed: false,
        }
    }

    pub fn from_days(p_val: u64) -> w_timespec_t {
        //convert days to milliseconds
        let _d = Duration::from_millis(p_val * 86400000u64);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_hours(p_val: u64) -> w_timespec_t {
        //convert hours to milliseconds
        let _d = Duration::from_millis(p_val * 3600000u64);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_mins(p_val: u64) -> w_timespec_t {
        //convert mins to milliseconds
        let _d = Duration::from_millis(p_val * 60000u64);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_secs(p_val: u64) -> w_timespec_t {
        w_timespec_t {
            secs: p_val,
            nano_seconds: 0,
            overflowed: false,
        }
    }

    pub fn from_millis(p_val: u64) -> w_timespec_t {
        let _d = Duration::from_millis(p_val);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_micros(p_val: u64) -> w_timespec_t {
        let _d = Duration::from_micros(p_val);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_nanos(p_val: u64) -> w_timespec_t {
        let _d = Duration::from_nanos(p_val);
        w_timespec_t {
            secs: _d.as_secs(),
            nano_seconds: _d.subsec_nanos(),
            overflowed: false,
        }
    }

    pub fn from_longtime(
        p_days: u64,
        p_hours: u64,
        p_mins: u64,
        p_secs: u64,
        p_millis: u64,
        p_micros: u64,
        p_nanos: u64,
    ) -> w_timespec_t {
        let _d_days_secs = Duration::from_secs(p_days * 86400u64);
        let _d_hrs_secs = Duration::from_secs(p_hours * 3600u64);
        let _d_mins_secs = Duration::from_secs(p_mins * 60u64);
        let _d_secs = Duration::from_secs(p_secs);
        let _d_millis = Duration::from_millis(p_millis);
        let _d_micros = Duration::from_micros(p_micros);
        let _d_nanos = Duration::from_nanos(p_nanos);

        let mut _ts = w_timespec_t {
            overflowed: true,
            ..Default::default()
        };

        let mut _t = _d_days_secs.checked_add(_d_hrs_secs);
        if _t.is_none() {
            return _ts;
        }
        _t = _t.unwrap().checked_add(_d_mins_secs);
        if _t.is_none() {
            return _ts;
        }
        _t = _t.unwrap().checked_add(_d_secs);
        if _t.is_none() {
            return _ts;
        }
        _t = _t.unwrap().checked_add(_d_millis);
        if _t.is_none() {
            return _ts;
        }
        _t = _t.unwrap().checked_add(_d_micros);
        if _t.is_none() {
            return _ts;
        }
        _t = _t.unwrap().checked_add(_d_nanos);
        if _t.is_none() {
            return _ts;
        }

        _ts.secs = _t.unwrap().as_secs();
        _ts.nano_seconds = _t.unwrap().subsec_nanos();
        _ts.overflowed = false;

        return _ts;
    }

    pub fn add(&mut self, p_val: &w_timespec_t) {
        let _self_secs = Duration::from_secs(self.secs);
        let _self_nanos = Duration::from_nanos(self.nano_seconds as u64);
        let _a = _self_secs.checked_add(_self_nanos);

        let _p_secs = Duration::from_secs(p_val.secs);
        let _p_nanos = Duration::from_nanos(p_val.nano_seconds as u64);
        let _b = _p_secs.checked_add(_p_nanos);

        match (_a, _b) {
            (Some(a), Some(b)) => {
                let _d = a.checked_add(b);
                if _d.is_some() {
                    let _ud = _d.unwrap();
                    self.secs = _ud.as_secs();
                    self.nano_seconds = _ud.subsec_nanos();
                    self.overflowed = false;
                }
            }
            _ => {
                self.overflowed = true;
            }
        };
    }

    pub fn sub(&mut self, p_val: &w_timespec_t) {
        let _self_secs = Duration::from_secs(self.secs);
        let _self_nanos = Duration::from_nanos(self.nano_seconds as u64);
        let _a = _self_secs.checked_add(_self_nanos);

        let _p_secs = Duration::from_secs(p_val.secs);
        let _p_nanos = Duration::from_nanos(p_val.nano_seconds as u64);
        let _b = _p_secs.checked_add(_p_nanos);

        match (_a, _b) {
            (Some(a), Some(b)) => {
                let _d = a.checked_sub(b);
                if _d.is_some() {
                    let _ud = _d.unwrap();
                    self.secs = _ud.as_secs();
                    self.nano_seconds = _ud.subsec_nanos();
                    self.overflowed = false;
                }
            }
            _ => {
                self.overflowed = true;
            }
        };
    }

    pub fn mul(&mut self, p_val: u32) {
        let _self_secs = Duration::from_secs(self.secs);
        let _self_nanos = Duration::from_nanos(self.nano_seconds as u64);
        let _a = _self_secs.checked_add(_self_nanos);

        match _a {
            Some(a) => {
                let _d = a.checked_mul(p_val);
                if _d.is_some() {
                    let _ud = _d.unwrap();
                    self.secs = _ud.as_secs();
                    self.nano_seconds = _ud.subsec_nanos();
                    self.overflowed = false;
                }
            }
            _ => {
                self.overflowed = true;
            }
        };
    }

    pub fn div(&mut self, p_val: u32) {
        let _self_secs = Duration::from_secs(self.secs);
        let _self_nanos = Duration::from_nanos(self.nano_seconds as u64);
        let _a = _self_secs.checked_add(_self_nanos);

        match _a {
            Some(a) => {
                let _d = a.checked_div(p_val);
                if _d.is_some() {
                    let _ud = _d.unwrap();
                    self.secs = _ud.as_secs();
                    self.nano_seconds = _ud.subsec_nanos();
                    self.overflowed = false;
                }
            }
            _ => {
                self.overflowed = true;
            }
        };
    }
}

/// unsigned 128 bit integer
#[repr(C)]
#[derive(Default)]
pub struct w_u128 {
    /// the upper bytes
    pub upper: u64,
    /// the lower bytes
    pub lower: u64,
}
/// Drop implementation for w_u128
impl Drop for w_u128 {
    fn drop(&mut self) {
        println!("dropping w_u128");
    }
}
//implementations
impl w_u128 {
    pub fn new(p_val: u128) -> w_u128 {
        w_u128 {
            upper: (p_val >> 64u128) as u64,
            lower: (p_val & 0xffffffffffffffffu128) as u64,
        }
    }
}

/// Convert total days to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
#[no_mangle]
pub extern "C" fn w_timespec_from_zero() -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_zero();
    Box::into_raw(Box::new(_ts))
}

/// Convert total days to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
#[no_mangle]
pub extern "C" fn w_timespec_from_days(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_days(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total hours to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
#[no_mangle]
pub extern "C" fn w_timespec_from_hours(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_hours(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total minutes to w_timespec_t
/// # Arguments
///
/// * `p_val` - minutes
#[no_mangle]
pub extern "C" fn w_timespec_from_mins(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_mins(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total seconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - seconds
#[no_mangle]
pub extern "C" fn w_timespec_from_secs(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_secs(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total milliseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - milliseconds
#[no_mangle]
pub extern "C" fn w_timespec_from_millis(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_millis(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total microseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - microseconds
#[no_mangle]
pub extern "C" fn w_timespec_from_micros(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_micros(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert total nanoseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - nanoseconds
#[no_mangle]
pub extern "C" fn w_timespec_from_nanos(p_val: u64) -> *mut w_timespec_t {
    let _ts = w_timespec_t::from_nanos(p_val);
    Box::into_raw(Box::new(_ts))
}

/// Convert long time to w_timespec_t
/// # Arguments
///
/// * `p_days` - days
/// * `p_hours` - hours
/// * `p_mins` - mins
/// * `p_secs` - seconds
/// * `p_millis` - milliseconds
/// * `p_micros` - microseconds
/// * `p_nanos` - nanoseconds
#[no_mangle]
pub extern "C" fn w_timespec_from_long_time(
    p_days: u64,
    p_hours: u64,
    p_mins: u64,
    p_secs: u64,
    p_millis: u64,
    p_micros: u64,
    p_nanos: u64,
) -> *mut w_timespec_t {
    let _ts =
        w_timespec_t::from_longtime(p_days, p_hours, p_mins, p_secs, p_millis, p_micros, p_nanos);
    Box::into_raw(Box::new(_ts))
}

/// pL += pR
/// # Arguments
///
/// * `p_lval` - left value
/// * `p_rval` - right value
#[no_mangle]
pub extern "C" fn w_timespec_add(
    p_lval: &mut *mut w_timespec_t,
    p_rval: *mut w_timespec_t,
) -> bool {
    if p_lval.is_null() || p_rval.is_null() {
        return false;
    }

    unsafe {
        let _l = p_lval.as_mut();
        let _r = p_rval.as_ref();
        match (_l, _r) {
            (Some(a), Some(b)) => {
                a.add(b);
                true
            }
            _ => false,
        }
    }
}

/// pL -= pR
/// # Arguments
///
/// * `p_lval` - left value
/// * `p_rval` - right value
#[no_mangle]
pub extern "C" fn w_timespec_sub(
    p_lval: &mut *mut w_timespec_t,
    p_rval: *mut w_timespec_t,
) -> bool {
    if p_lval.is_null() || p_rval.is_null() {
        return false;
    }

    unsafe {
        let _l = p_lval.as_mut();
        let _r = p_rval.as_ref();
        match (_l, _r) {
            (Some(a), Some(b)) => {
                a.sub(b);
                true
            }
            _ => false,
        }
    }
}

/// pL *= pR
/// # Arguments
///
/// * `p_lval` - left value
/// * `p_rval` - right unsigned integer value
#[no_mangle]
pub extern "C" fn w_timespec_mul(p_lval: &mut *mut w_timespec_t, p_rval: u32) -> bool {
    if p_lval.is_null() {
        return false;
    }

    unsafe {
        let _l = p_lval.as_mut();
        match _l {
            Some(a) => {
                a.mul(p_rval);
                true
            }
            _ => false,
        }
    }
}

/// pL *= pR
/// # Arguments
///
/// * `p_lval` - left value
/// * `p_rval` - right unsigned integer value
#[no_mangle]
pub extern "C" fn w_timespec_div(p_lval: &mut *mut w_timespec_t, p_rval: u32) -> bool {
    if p_lval.is_null() {
        return false;
    }

    unsafe {
        let _l = p_lval.as_mut();
        match _l {
            Some(a) => {
                a.div(p_rval);
                true
            }
            _ => false,
        }
    }
}


//TODOs:
//timespec to string
//timespec from string
//timespec getters days, hours,...