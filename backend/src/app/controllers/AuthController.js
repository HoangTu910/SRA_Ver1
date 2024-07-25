const { loginUser,
    logout,
    signUpUser
} = require('../services/AuthService');

class AuthController {
    loginController = async (req, res) => {
        const { token } = req.body;
      
        try {
          if (!token) {
            return res.status(400).json({ message: 'Token not provided' });
          }
      
          const user = await loginUser(token);
          res.status(200).json(user);
        } catch (error) {
          res.status(500).json({ message: error.message });
        }
    };

    logOut = async (req, res) => {
        try {
            await logout();
            res.status(200).json("Logged out");
        } catch (error) {
            console.error("Logout failed: ", error.message);
            res.status(500).json({ message: error.message });
        }
    }

    signUp = async(req, res) => {
        try{
            const {email, password} = req.body
            const response = await signUpUser(email, password)
            return res.status(201).json({message: `Create user successfully: ${response.user}`})
        }
        catch (error){
            console.error("Sign up failed: ", error.message);
            res.status(500).json({ message: error.message });
        }
    }
}

module.exports = new AuthController();