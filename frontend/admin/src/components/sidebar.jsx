import React from 'react';
import { Box, List, ListItem, ListItemIcon, ListItemText } from '@mui/material';
import HomeIcon from '@mui/icons-material/Home';
import DevicesIcon from '@mui/icons-material/Devices';
import MedicalServicesIcon from '@mui/icons-material/MedicalServices';
import HealthAndSafetyIcon from '@mui/icons-material/HealthAndSafety';
import { styled } from '@mui/material/styles';

const CustomListItemText = styled(ListItemText)(({ theme }) => ({
  '& .MuiListItemText-primary': {
      fontFamily: 'Consolas, sans-serif',
      fontSize: '14px',
      fontWeight: 'bold',
      color: 'black' 
  },
}));

const buttonStyle = () => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  width: '80px',
});

const sidebarStyle = () => ({
  display: 'flex',
  height: 'auto', // Make the sidebar take up the full viewport height
  width: '180px', // Adjust the width as needed
  bgcolor: '#d6eaf8', // Background color
  color: '#fff', // Text color

});

const listStyle = () => ({
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'space-between',
  flex: 1, // Make the list take up remaining vertical space
});

const innerBox = () => ({
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'space-between',
  flex: 1, // Make the list take up remaining vertical space
  height: '300px'
});

const Sidebar = () => {
  return (
    <Box sx={sidebarStyle()}>
      <Box sx={innerBox()}>
        <List sx={listStyle()}>
          <ListItem button>
            <ListItemIcon style={buttonStyle()}>
              <HomeIcon style={{ color: '#5dade2' }} />
            </ListItemIcon>
          </ListItem>
          <ListItem button>
            <ListItemIcon style={buttonStyle()}>
              <DevicesIcon style={{ color: '#5dade2' }} />
            </ListItemIcon>
          </ListItem>
          <ListItem button>
            <ListItemIcon style={buttonStyle()}>
              <MedicalServicesIcon style={{ color: '#5dade2' }} />
            </ListItemIcon>
          </ListItem>
          <ListItem button>
            <ListItemIcon style={buttonStyle()}>
              <HealthAndSafetyIcon style={{ color: '#5dade2' }} />
            </ListItemIcon>
          </ListItem>
        </List>
      </Box>
    </Box>
  );
};

export default Sidebar;
