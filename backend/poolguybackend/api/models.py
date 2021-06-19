import uuid

from django.contrib.auth.models import User
from django.db import models
from django.db.models import fields, ForeignKey, ManyToManyField


class Device(models.Model):
    """
    Represents a Poolguy device.
    All Poolguy devices have a unique device_id
    property, hard-coded in firmware from their
    flashing provisioning which is referred to
    here. If it is not provided yet, meaning an
    instance of this model represents a new Poolguy
    device - it is provided a uuid4 string to
    identify with.
    """
    owner = ForeignKey(User, on_delete=models.CASCADE)
    device_id = fields.CharField(max_length=255, default=uuid.uuid4())
    given_name = fields.TextField(max_length=255, default="Poolguy Device")


class DeviceMessage(models.Model):
    """
    Represents a datamessage sent from
    a Poolguy device. The message contains
    information about temperature in float,
    the unit (Celcius vs. Fahr..), and
    battery percentage (0-100).
    """

    temperature_value = fields.FloatField(default=0.0)
    battery_level = fields.IntegerField(default=0)
    unit = fields.CharField(default="C", max_length=255)
    device = ManyToManyField(Device)

