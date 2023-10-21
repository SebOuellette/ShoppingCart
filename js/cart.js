// This file contains the cart object and relative tasks
//(()=>{

    class Cart {
        constructor() {
            // this.x = 1
            // this.y = 2


        }


        getProfileLink(profile) {
            let domain = document.location.hostname;
            let profileID = profile.getProfileID();

            return `https://${domain}/users/${profileID}`;
        }
    }

//});