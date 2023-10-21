// This file contains the cart object and relative tasks
//(()=>{

    class Cart {
        constructor() {
            // this.x = 1
            // this.y = 2


        }


        getProfileLink(profileID) {
            let domain = document.location.hostname;

            return `https://${domain}/users/${profileID}`;
        }
    }

//});