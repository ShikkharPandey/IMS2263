document.addEventListener("DOMContentLoaded", function () {
    document.getElementById("login-form").addEventListener("submit", function (e) {
        e.preventDefault();

        const username = document.getElementById("username").value.trim();
        const password = document.getElementById("password").value;

        fetch("/login", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username, password })
        })
        .then(res => res.text())
        .then(text => {
            if (text === "success") {
                window.location.href = "/index.html";
            } else {
                document.getElementById("message").innerText = "Invalid credentials.";
            }
        });
    });
});
