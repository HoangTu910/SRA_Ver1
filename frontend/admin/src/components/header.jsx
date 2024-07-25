import React from 'react';
import { AppBar, Toolbar, Typography, InputBase, Box } from '@mui/material';
import SearchIcon from '@mui/icons-material/Search';

const Header = () => {
  return (
    <AppBar position="static" color="transparent" elevation={0}>
      <Toolbar>
        <Box sx={{ flexGrow: 1 }}>
          <Box display="flex" alignItems="center">
            <SearchIcon />
            <InputBase placeholder="Search Doctors" sx={{ ml: 1, flex: 1 }} />
          </Box>
        </Box>
        <Box>
          <Typography variant="h6" noWrap component="div">
            User
          </Typography>
        </Box>
      </Toolbar>
    </AppBar>
  );
};

export default Header;
