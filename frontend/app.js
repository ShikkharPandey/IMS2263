document.addEventListener("DOMContentLoaded", function () {
    loadInventory();
});



/**
 * This function fetches the current inventory data from the server and updates the table in the UI.
 * It sends a GET request to the "/inventory" endpoint, parses the JSON response, and populates the inventory table.
 * If there's an error during the fetch or parsing process, it logs the error to the console.
 */

function loadInventory() {
    fetch("/inventory")
        .then(response => {
            if (!response.ok) throw new Error(`HTTP error! Status: ${response.status}`);
            return response.text();  // Read as text first
        })
        .then(text => {
            console.log("Raw JSON response:", text); // Debugging log
            return JSON.parse(text);  // Now parse JSON
        })
        .then(data => {
            console.log("Fetched inventory data:", data);
            const tableBody = document.querySelector("#inventory-table tbody");
            tableBody.innerHTML = "";

            data.forEach(item => {
                let row = document.createElement("tr");
                row.innerHTML = `
                    <td>${item.id}</td>
                    <td>${item.name}</td>
                    <td>${item.quantity}</td>
                    <td>$${item.price}</td>
                    <td class='actions'>
                        <button onclick="editItem(${item.id})">Edit</button>
                        <button onclick="deleteItem(${item.id})">Delete</button>
                    </td>
                `;
                tableBody.appendChild(row);
            });
        })
        .catch(error => console.error("Error loading inventory:", error));
}


function addItem() {
    let name = prompt("Enter item name:");
    let quantity = parseInt(prompt("Enter quantity:"));
    let price = parseFloat(prompt("Enter price:"));

    if (!name || isNaN(quantity) || isNaN(price)) {
        alert("Invalid input! Please enter valid values.");
        return;
    }

    let newItem = {
        id: Date.now(),
        name: name,
        quantity: quantity,
        price: price
    };

    fetch("/inventory", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(newItem)
    })
    .then(response => response.json())
    .then(data => {
        console.log("Updated inventory:", data);
        loadInventory(); // Reload the table after adding an item
    })
    .catch(error => console.error("Error adding item:", error));
}

function deleteItem(itemId) {
    console.log(`Deleting item with id ${itemId}`);
  
    fetch(`/inventory?id=${itemId}`, { // Ensure backend supports query params
      method: "DELETE"
    })
    .then(response => response.json())
    .then(data => {
      console.log("Item deleted successfully!", data);
      loadInventory(); // Reload the table after deleting an item
    })
    .catch(error => console.error("Error deleting item:", error));
  }
  