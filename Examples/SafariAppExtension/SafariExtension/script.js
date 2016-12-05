/*
 * Safari Token Signing
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

document.addEventListener("DOMContentLoaded", function(event) {
    console.log("DOMContentLoaded");
    safari.self.addEventListener("message", function(event) {
        console.log(event.name);
        console.log(event.message);

        if (event.name == "cert") {
            var newElement = document.createElement("div");
            newElement.innerHTML = event.message.cert;
            document.body.insertBefore(newElement, document.body.firstChild);

            var digest = document.getElementById("digest");
            console.log("Digest content: ", digest.innerHTML);
            safari.extension.dispatchMessage("digest", {
                "digest": digest.innerHTML,
                "algo": "SHA1"
            });
        } else if (event.name == "signature") {
            var newElement = document.createElement("div");
            newElement.innerHTML = event.message.signature;
            document.body.insertBefore(newElement, document.body.firstChild);
        }
    });
    safari.extension.dispatchMessage("cert");
});
