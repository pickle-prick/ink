-- Detect Environment
local is_wsl = vim.fn.has("wsl") == 1
local is_win = vim.fn.has("win32") == 1

if is_win then
  -- Force Neovim to use CMD for background commands (like :make)
  vim.o.shell = 'cmd.exe'
  vim.o.shellcmdflag = '/s /c'

  -- Standard CMD redirection (works perfectly with .bat files)
  vim.o.shellpipe = '>%s 2>&1' 
  vim.o.shellquote = ''
  vim.o.shellxquote = '"'

  -- Point directly to your script
  vim.o.makeprg = 'build.bat'
end

if is_wsl then
  vim.o.makeprg = "./build_wsl.sh"
  vim.opt.errorformat = {
    '%f(%l) : %t%*[^ ] %m',  -- Match with space before colon
    '%f(%l): %t%*[^ ] %m',   -- Match without space before colon
    '%f(%l) : %m',           -- Generic match with space
    '%f(%l): %m',            -- Generic match without space
    '%f:%l:%c:%t:%m',        -- Standard formats
    '%f:%l:%t:%m',
  }
end

vim.keymap.set('n', '<F5>', '<cmd>Make ink msvc<CR>', { desc = 'Run async Make' })
vim.keymap.set('n', '<F6>', '<cmd>Make ink msvc run<CR>', { desc = 'Run async Make' })
vim.keymap.set('n', '<leader>gg', '<cmd>Make<CR>', { desc = 'Run async Make' })
