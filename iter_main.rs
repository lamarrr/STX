use std::ops::Div;
use std::option;

pub fn inlined_func(array: &[f32], scaling: f32) -> f32 {
    array
        .iter()
        .map(|x| *x * scaling)
        .map(|x| x + 1.98765432)
        .fold(0_f32, |sum, x| sum + x)
        .div(array.len() as f32)
}

pub fn scale(array: &mut [f32], scaling: f32) {
    array.iter_mut().for_each(|x| {
        *x = *x * scaling;
    })
}

pub fn offset(array: &mut [f32], y: f32) {
    array.iter_mut().for_each(|x| {
        *x = *x + y;
    })
}

pub fn mean(array: &[f32]) -> f32 {
    let sum = array.iter().fold(0_f32, |sum, x| sum + x);
    sum / array.len() as f32
}

pub fn composed_func(array: &mut [f32], scaling: f32) -> f32 {
    scale(array, scaling);
    offset(array, 1.98765432f32);
    mean(array)
}

pub struct Y {
 x: i32,
 y: i32
}

pub fn main() {
    let ff :Option<Y> = None;
    ff.is_some();

    let g:Result<i32,i32> = OK(9);

    g.is_err()
    let _c = Y{x:65,y:89};
    match ff {
        Some(_c) => {println!("{}",9);} ,
        Some(_c) => {println!("{}>>", 9);},
        Some(_c) => {println!{"{} ", 9}; },
        None => {}
    };
    let mut arr_a = [3.4, 5.7, 9.0];
    let arr_b = [3.4, 5.7, 9.0];
    println!(
        "{} {}",
        composed_func(&mut arr_a, 0.6),
        inlined_func(&arr_b, 0.6)
    )
    option::exp
}
