#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
#[no_mangle]
pub extern fn test(){
    println!("hello world");
}