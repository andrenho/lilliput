" files format
set fileformat=unix

set ts=4
set sts=4
set sw=4
set expandtab
set foldmethod=marker

" open all files
e index.html
args .vimrc
b 1
map <Leader>V   :b .vimrc<CR>
