// import { useState } from 'react'
// import Login from "./pages/login.jsx"
// import AppPage from "./pages/dashboard.jsx"
// import { BrowserRouter as Router, Route, Routes } from "react-router-dom";

// function App() {
//   const [count, setCount] = useState(0)

//   return (
//     <Router>
//       <Routes>
//         <Route path="/login" element={<Login/>}></Route>
//         <Route path="/dashboard" element={<AppPage/>}></Route>
//       </Routes>
//     </Router>
//   );
// }

// export default App

/* eslint-disable perfectionist/sort-imports */
import './global.css';

import { useScrollToTop } from './hooks/use-scroll-to-top';

import Router from './routes/sections';
import ThemeProvider from './theme';

// ----------------------------------------------------------------------

export default function App() {
  useScrollToTop();

  return (
    <ThemeProvider>
      <Router />
    </ThemeProvider>
  );
}