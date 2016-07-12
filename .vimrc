" files format
set fileformat=unix

set ts=4
set sts=4
set sw=4
set expandtab
set foldmethod=marker

" make program
set makeprg=clear\ \&\&\ make
nnoremap <Leader>B :!clear && cargo build<cr>
nnoremap <Leader>T :!clear && cargo test<cr>
nnoremap <Leader>R :!clear && cargo run<cr>
nnoremap <Leader>G :!git commit -a && git push<CR>

" open all files
e src/main.rs
args src/*.rs
args Cargo.toml
args .vimrc
b 1
map <Leader>x   :b src/main.rs<CR>
map <Leader>c   :b src/cpu.rs<CR>
map <Leader>m   :b src/computer.rs<CR>
map <Leader>d   :b src/device.rs<CR>
map <Leader>C   :b Cargo.toml<CR>
map <Leader>V   :b .vimrc<CR>

" swap between cc and hh
function! SwapBuffers()
  if expand("%:e") == "c"
    exe "b" fnameescape(expand("%:r").".h")
  elseif expand("%:e") == "h"
    exe "b" fnameescape(expand("%:r").".c")
  endif
endfunction
map <Leader>.  :call SwapBuffers()<CR>
