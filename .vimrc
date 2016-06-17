" files format
set fileformat=unix

set ts=4
set sts=4
set sw=4
set expandtab

" make program
set makeprg=clear\ \&\&\ make
nnoremap <Leader>b :make! debug<cr>
nnoremap <Leader>B :make! debug FORCE_COLOR=1<cr>
nnoremap <Leader>L :make! debug CXX=clang++<cr>
nnoremap <Leader>C :make! clean<cr>
nnoremap <Leader>r :!clear && ./lilliput<cr>
nnoremap <Leader>G :!git commit -a && git push<CR>

" open all files
e src/main.c
args Makefile
args src/*.h
args src/*.c
args .vimrc
b 1   " open initial buffer

" open specific buffers
"map <Leader>c   :b src/components.hh<CR>
"map <Leader>g   :b src/gamedata.hh<CR>
"map <Leader>p   :b src/physics.cc<CR>
map <Leader>m   :b src/main.c<CR>
"map <Leader>u   :b src/uidebug.cc<CR>
map <Leader>M   :b Makefile<CR>
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
